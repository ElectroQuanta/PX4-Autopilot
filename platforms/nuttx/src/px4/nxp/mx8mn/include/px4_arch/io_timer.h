/****************************************************************************
 *
 *   Copyright (C) 2024 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file io_timer.h
 *
 * i.MX8MN PWM output data.
 */
#pragma once

#include <px4_platform_common/px4_config.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <drivers/drv_hrt.h>

__BEGIN_DECLS

/* Configuration limits */

#ifdef BOARD_NUM_IO_TIMERS
#define MAX_IO_TIMERS			BOARD_NUM_IO_TIMERS
#else
#define MAX_IO_TIMERS			4  /* PWM1-4 */
#endif

#define MAX_TIMER_IO_CHANNELS		4  /* One channel per PWM module */

#define MAX_LED_TIMERS			0  /* No dedicated LED timers yet */
#define MAX_TIMER_LED_CHANNELS		0

#define IO_TIMER_ALL_MODES_CHANNELS	0

/* Channel modes */

typedef enum io_timer_channel_mode_t {
	IOTimerChanMode_NotUsed = 0,
	IOTimerChanMode_PWMOut  = 1,
	IOTimerChanMode_PWMIn   = 2,
	IOTimerChanMode_Capture = 3,
	IOTimerChanMode_OneShot = 4,
	IOTimerChanMode_Trigger = 5,
	IOTimerChanMode_Dshot   = 6,
	IOTimerChanMode_LED     = 7,
	IOTimerChanMode_PPS     = 8,
	IOTimerChanMode_RPM     = 9,
	IOTimerChanMode_Other   = 10,
	IOTimerChanModeSize
} io_timer_channel_mode_t;

typedef uint16_t io_timer_channel_allocation_t; /* big enough to hold MAX_TIMER_IO_CHANNELS */

/* Timer configuration structure
 * Note: For i.MX8MN, each PWM module is a separate timer with one channel.
 * The base address points to the PWM module registers.
 * The clock_freq is set to 1MHz after prescaling for standard PWM mode,
 * or 8MHz for OneShot mode.
 */
typedef struct io_timers_t {
	uint32_t	base;                /* Base address of PWM module */
	uint32_t	clock_register;      /* CCM clock gate index */
	uint32_t	clock_bit;           /* Unused for i.MX8MN */
	uint32_t	vectorno;            /* IRQ number (unused for now) */
} io_timers_t;

/* Channel mapping structure */

typedef struct io_timers_channel_mapping_element_t {
	uint32_t first_channel_index;
	uint32_t channel_count;
	uint32_t lowest_timer_channel;
	uint32_t channel_count_including_gaps;
} io_timers_channel_mapping_element_t;

typedef struct io_timers_channel_mapping_t {
	io_timers_channel_mapping_element_t element[MAX_IO_TIMERS];
} io_timers_channel_mapping_t;

/* Channel configuration structure */

typedef struct timer_io_channels_t {
	uint32_t	gpio_out;            /* IOMUX pin configuration for PWM output */
	uint32_t	gpio_in;             /* IOMUX pin configuration for capture (unused) */
	uint8_t		timer_index;         /* 0-based index in io_timers_t table */
	uint8_t		timer_channel;       /* Always 1 for i.MX8MN (one channel per PWM) */
} timer_io_channels_t;

/* Channel handler callback */

typedef void (*channel_handler_t)(void *context, const io_timers_t *timer, uint32_t chan_index,
				  const timer_io_channels_t *chan,
				  hrt_abstime isrs_time, uint16_t isrs_rcnt,
				  uint16_t capture);

/* Board-specific data (supplied by board-specific code) */

__EXPORT extern const io_timers_t io_timers[MAX_IO_TIMERS];
__EXPORT extern const io_timers_channel_mapping_t io_timers_channel_mapping;
__EXPORT extern const timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS];

__EXPORT extern const io_timers_t led_pwm_timers[MAX_LED_TIMERS];
__EXPORT extern const timer_io_channels_t led_pwm_channels[MAX_TIMER_LED_CHANNELS];

__EXPORT extern io_timer_channel_allocation_t allocations[IOTimerChanModeSize];

/* Public API */

__EXPORT int io_timer_channel_init(unsigned channel, io_timer_channel_mode_t mode,
				   channel_handler_t channel_handler, void *context);

__EXPORT int io_timer_init_timer(unsigned timer, io_timer_channel_mode_t mode);

__EXPORT int io_timer_set_pwm_rate(unsigned timer, unsigned rate);

__EXPORT int io_timer_set_enable(bool state, io_timer_channel_mode_t mode,
				 io_timer_channel_allocation_t masks);

__EXPORT uint16_t io_channel_get_ccr(unsigned channel);

__EXPORT int io_timer_set_ccr(unsigned channel, uint16_t value);

__EXPORT uint32_t io_timer_get_group(unsigned timer);

__EXPORT int io_timer_validate_channel_index(unsigned channel);

__EXPORT int io_timer_allocate_channel(unsigned channel, io_timer_channel_mode_t mode);

__EXPORT int io_timer_unallocate_channel(unsigned channel);

__EXPORT int io_timer_get_channel_mode(unsigned channel);

__EXPORT int io_timer_get_mode_channels(io_timer_channel_mode_t mode);

__EXPORT extern void io_timer_trigger(unsigned channel_mask);

__EXPORT int io_timer_allocate_timer(unsigned timer, io_timer_channel_mode_t mode);

__EXPORT int io_timer_unallocate_timer(unsigned timer);

__EXPORT uint32_t io_timer_channel_get_gpio_output(unsigned channel);

__EXPORT uint32_t io_timer_channel_get_as_pwm_input(unsigned channel);

__END_DECLS
