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

/*
 * @file timer_config.cpp
 *
 * Configuration data for the i.MX8MN PWM driver.
 *
 * Timer/Channel Mapping:
 *   PWM1/Channel1 -> SPDIF_EXT_CLK (Motor 1)
 *   PWM2/Channel1 -> SPDIF_RX      (Motor 2)
 *   PWM3/Channel1 -> SPDIF_TX      (Motor 3)
 *   PWM4/Channel1 -> SAI3_MCLK     (Motor 4)
 */

#include <stdint.h>

#include <drivers/drv_pwm_output.h>
#include <px4_arch/io_timer_hw_description.h>

#include "board_config.h"

/* Timer configuration array */

constexpr io_timers_t io_timers[MAX_IO_TIMERS] = {
	initIOTimer(Timer::PWM1),
	initIOTimer(Timer::PWM2),
	initIOTimer(Timer::PWM3),
	initIOTimer(Timer::PWM4),
};

/* Channel configuration array
 * Note: For i.MX8MN, each PWM module has only one output channel.
 * The GPIO pin configuration is handled via IOMUXC.
 */

constexpr timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
	// Motor 1: PWM1 on GPIO1_IO01
	initIOTimerChannel(io_timers, {Timer::PWM1, Timer::Channel1}, {GPIO::PortInvalid, GPIO::Pin0}),

	// Motor 2: PWM2 on GPIO1_IO13
	initIOTimerChannel(io_timers, {Timer::PWM2, Timer::Channel1}, {GPIO::PortInvalid, GPIO::Pin0}),

	// Motor 3: PWM3 on GPIO1_IO10
	initIOTimerChannel(io_timers, {Timer::PWM3, Timer::Channel1}, {GPIO::PortInvalid, GPIO::Pin0}),

	// Motor 4: PWM4 on SAI3_MCLK
	initIOTimerChannel(io_timers, {Timer::PWM4, Timer::Channel1}, {GPIO::PortInvalid, GPIO::Pin0}),
};

/* Channel mapping initialization */

constexpr io_timers_channel_mapping_t io_timers_channel_mapping =
	initIOTimerChannelMapping(io_timers, timer_io_channels);

/* LED PWM timers (not used on mx8mn yet) */

const struct io_timers_t led_pwm_timers[MAX_LED_TIMERS] = {
};

const struct timer_io_channels_t led_pwm_channels[MAX_TIMER_LED_CHANNELS] = {
};

/****************************************************************************
 * Name: mx8mn_timer_initialize
 *
 * Description:
 *   Initialize the HRT (High-Resolution Timer) subsystem.
 *   This function is called during board initialization to set up GPT1
 *   for 1 MHz operation as PX4's time base.
 *
 ****************************************************************************/

#include <drivers/drv_hrt.h>

void mx8mn_timer_initialize(void)
{
	/* Initialize the HRT driver - this configures GPT1 */
	hrt_init();
}
