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
 * @file io_timer.c
 *
 * Servo driver supporting PWM servos connected to i.MX8MN PWM modules.
 */

#include <px4_platform_common/px4_config.h>
#include <systemlib/px4_macros.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <sys/types.h>
#include <stdbool.h>

#include <assert.h>
#include <debug.h>
#include <time.h>
#include <queue.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <arch/board/board.h>
#include <drivers/drv_pwm_output.h>

#include <px4_arch/io_timer.h>

#include "hardware/mx8mn_memorymap.h"
#include "hardware/mx8mn_pwm.h"
#include "hardware/mx8mn_ccm.h"
#include "hardware/mx8mn_pinmux.h"
#include "mx8mn_pwm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* PWM frequencies for different modes */

#if !defined(BOARD_PWM_FREQ)
#define BOARD_PWM_FREQ		400		/* 400 Hz for standard ESC */
#endif

#if !defined(BOARD_ONESHOT_FREQ)
#define BOARD_ONESHOT_FREQ	8000000		/* 8 MHz for OneShot125 */
#endif

/* Conversion between PX4's 1MHz time base and actual timer values */

#define USEC_TO_PERIOD_US(x)	(x)

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Channel context for interrupt handlers */

struct channel_stat_t {
	channel_handler_t	callback;
	void			*context;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Allocation tracking */

io_timer_channel_allocation_t allocations[IOTimerChanModeSize] = { };

/* Channel interrupt contexts */

static struct channel_stat_t channel_handlers[MAX_TIMER_IO_CHANNELS];

/* Current PWM rate per timer (Hz) */

static uint32_t timer_rates[MAX_IO_TIMERS];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: get_timer_channels
 *
 * Description:
 *   Get channel count for a timer
 *
 ****************************************************************************/

static inline unsigned get_timer_channels(unsigned timer)
{
	return io_timers_channel_mapping.element[timer].channel_count;
}

/****************************************************************************
 * Name: validate_timer_index
 *
 * Description:
 *   Validate timer index
 *
 ****************************************************************************/

static inline int validate_timer_index(unsigned timer)
{
	return (timer < MAX_IO_TIMERS) ? 0 : -EINVAL;
}

/****************************************************************************
 * Name: pwm_timer_init
 *
 * Description:
 *   Initialize a PWM timer module with pin configuration from board
 *
 ****************************************************************************/

static int pwm_timer_init(unsigned timer)
{
	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	/* Get pin configuration from timer_io_channels.
	 * This allows the board-level configuration (timer_config.cpp)
	 * to specify which pins to use via board.h defines.
	 */

	uint32_t pin = timer_io_channels[timer].gpio_out;

	/* Initialize the NuttX PWM driver with the specified pin.
	 * Pass 0 to use the default pin from board.h, or pass a specific
	 * IOMUXC constant to override at runtime.
	 */

	return mx8mn_pwm_init_with_pin(timer + 1, pin);  /* PWM IDs are 1-based */
}

/****************************************************************************
 * Name: pwm_timer_set_rate
 *
 * Description:
 *   Set PWM frequency for a timer
 *
 ****************************************************************************/

static int pwm_timer_set_rate(unsigned timer, unsigned rate)
{
	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	/* Configure PWM frequency */

	int ret = mx8mn_pwm_configure(timer + 1, rate, MX8MN_PWM_POLARITY_NORMAL);

	if (ret == 0) {
		timer_rates[timer] = rate;
	}

	return ret;
}

/****************************************************************************
 * Name: pwm_timer_set_pulse
 *
 * Description:
 *   Set PWM pulse width in microseconds
 *
 ****************************************************************************/

static int pwm_timer_set_pulse(unsigned timer, unsigned pulse_us)
{
	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	/* Set duty cycle in microseconds */

	return mx8mn_pwm_set_duty_cycle(timer + 1, pulse_us);
}

/****************************************************************************
 * Name: pwm_timer_enable
 *
 * Description:
 *   Enable/disable PWM output
 *
 ****************************************************************************/

static int pwm_timer_enable(unsigned timer, bool enable)
{
	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	return mx8mn_pwm_enable(timer + 1, enable);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: io_timer_validate_channel_index
 ****************************************************************************/

int io_timer_validate_channel_index(unsigned channel)
{
	return (channel < MAX_TIMER_IO_CHANNELS) ? 0 : -EINVAL;
}

/****************************************************************************
 * Name: io_timer_allocate_timer
 ****************************************************************************/

int io_timer_allocate_timer(unsigned timer, io_timer_channel_mode_t mode)
{
	int ret = validate_timer_index(timer);

	if (ret != 0) {
		return ret;
	}

	/* Check if already allocated to this mode */

	if (allocations[mode] & (1 << timer)) {
		return 0;
	}

	/* Check if allocated to another mode */

	for (unsigned m = 0; m < IOTimerChanModeSize; m++) {
		if (m != mode && (allocations[m] & (1 << timer))) {
			return -EBUSY;
		}
	}

	/* Mark as allocated */

	allocations[mode] |= (1 << timer);

	return 0;
}

/****************************************************************************
 * Name: io_timer_unallocate_timer
 ****************************************************************************/

int io_timer_unallocate_timer(unsigned timer)
{
	int ret = validate_timer_index(timer);

	if (ret != 0) {
		return ret;
	}

	/* Clear allocation for all modes */

	for (unsigned mode = 0; mode < IOTimerChanModeSize; mode++) {
		allocations[mode] &= ~(1 << timer);
	}

	return 0;
}

/****************************************************************************
 * Name: io_timer_allocate_channel
 ****************************************************************************/

int io_timer_allocate_channel(unsigned channel, io_timer_channel_mode_t mode)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return -EINVAL;
	}

	/* For i.MX8MN, channel index == timer index */

	unsigned timer = timer_io_channels[channel].timer_index;

	return io_timer_allocate_timer(timer, mode);
}

/****************************************************************************
 * Name: io_timer_unallocate_channel
 ****************************************************************************/

int io_timer_unallocate_channel(unsigned channel)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return -EINVAL;
	}

	unsigned timer = timer_io_channels[channel].timer_index;

	return io_timer_unallocate_timer(timer);
}

/****************************************************************************
 * Name: io_timer_get_channel_mode
 ****************************************************************************/

int io_timer_get_channel_mode(unsigned channel)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return IOTimerChanMode_NotUsed;
	}

	unsigned timer = timer_io_channels[channel].timer_index;

	for (unsigned mode = 0; mode < IOTimerChanModeSize; mode++) {
		if (allocations[mode] & (1 << timer)) {
			return mode;
		}
	}

	return IOTimerChanMode_NotUsed;
}

/****************************************************************************
 * Name: io_timer_get_mode_channels
 ****************************************************************************/

int io_timer_get_mode_channels(io_timer_channel_mode_t mode)
{
	if (mode >= IOTimerChanModeSize) {
		return 0;
	}

	return allocations[mode];
}

/****************************************************************************
 * Name: io_timer_init_timer
 ****************************************************************************/

int io_timer_init_timer(unsigned timer, io_timer_channel_mode_t mode)
{
	int ret;

	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	/* Allocate timer */

	ret = io_timer_allocate_timer(timer, mode);

	if (ret != 0) {
		return ret;
	}

	/* Initialize PWM hardware */

	ret = pwm_timer_init(timer);

	if (ret != 0) {
		io_timer_unallocate_timer(timer);
		return ret;
	}

	/* Set default PWM rate based on mode */

	unsigned rate;

	switch (mode) {
	case IOTimerChanMode_PWMOut:
		rate = BOARD_PWM_FREQ;
		break;

	case IOTimerChanMode_OneShot:
		rate = BOARD_ONESHOT_FREQ;
		break;

	default:
		rate = BOARD_PWM_FREQ;
		break;
	}

	ret = pwm_timer_set_rate(timer, rate);

	if (ret != 0) {
		io_timer_unallocate_timer(timer);
		return ret;
	}

	return 0;
}

/****************************************************************************
 * Name: io_timer_set_pwm_rate
 ****************************************************************************/

int io_timer_set_pwm_rate(unsigned timer, unsigned rate)
{
	if (validate_timer_index(timer) != 0) {
		return -EINVAL;
	}

	/* Validate rate */

	if (rate == 0 || rate > 50000) {
		return -EINVAL;
	}

	return pwm_timer_set_rate(timer, rate);
}

/****************************************************************************
 * Name: io_timer_channel_init
 ****************************************************************************/

int io_timer_channel_init(unsigned channel, io_timer_channel_mode_t mode,
			   channel_handler_t channel_handler, void *context)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return -EINVAL;
	}

	unsigned timer = timer_io_channels[channel].timer_index;

	int ret = io_timer_init_timer(timer, mode);

	if (ret != 0) {
		return ret;
	}

	/* Save callback context */

	channel_handlers[channel].callback = channel_handler;
	channel_handlers[channel].context = context;

	return 0;
}

/****************************************************************************
 * Name: io_timer_set_ccr
 ****************************************************************************/

int io_timer_set_ccr(unsigned channel, uint16_t value)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return -EINVAL;
	}

	unsigned timer = timer_io_channels[channel].timer_index;

	/* Convert from PX4's 1MHz time base to microseconds */

	uint32_t pulse_us = USEC_TO_PERIOD_US(value);

	return pwm_timer_set_pulse(timer, pulse_us);
}

/****************************************************************************
 * Name: io_channel_get_ccr
 ****************************************************************************/

uint16_t io_channel_get_ccr(unsigned channel)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return 0;
	}

	/* For i.MX8MN, we don't have a read-back mechanism yet.
	 * Return 0 to indicate unknown value.
	 */

	return 0;
}

/****************************************************************************
 * Name: io_timer_set_enable
 ****************************************************************************/

int io_timer_set_enable(bool state, io_timer_channel_mode_t mode,
			io_timer_channel_allocation_t masks)
{
	int ret = 0;

	/* Process each timer bit in the mask */

	for (unsigned timer = 0; timer < MAX_IO_TIMERS; timer++) {
		if (masks & (1 << timer)) {
			int rv = pwm_timer_enable(timer, state);

			if (rv != 0) {
				ret = rv;
			}
		}
	}

	return ret;
}

/****************************************************************************
 * Name: io_timer_get_group
 ****************************************************************************/

uint32_t io_timer_get_group(unsigned timer)
{
	if (validate_timer_index(timer) != 0) {
		return 0;
	}

	/* For i.MX8MN, each PWM is independent, so return timer bit */

	return (1 << timer);
}

/****************************************************************************
 * Name: io_timer_trigger
 ****************************************************************************/

void io_timer_trigger(unsigned channel_mask)
{
	/* i.MX8MN PWM doesn't support synchronized triggering */
}

/****************************************************************************
 * Name: io_timer_channel_get_gpio_output
 ****************************************************************************/

uint32_t io_timer_channel_get_gpio_output(unsigned channel)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return 0;
	}

	return timer_io_channels[channel].gpio_out;
}

/****************************************************************************
 * Name: io_timer_channel_get_as_pwm_input
 ****************************************************************************/

uint32_t io_timer_channel_get_as_pwm_input(unsigned channel)
{
	if (io_timer_validate_channel_index(channel) != 0) {
		return 0;
	}

	return timer_io_channels[channel].gpio_in;
}
