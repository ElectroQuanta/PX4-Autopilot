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

#pragma once

#include <px4_arch/io_timer.h>
#include <px4_arch/hw_description.h>
#include <px4_platform_common/constexpr_util.h>
#include <px4_platform_common/px4_config.h>
#include <px4_platform/io_timer_init.h>

#include "hardware/mx8mn_memorymap.h"
#include "hardware/mx8mn_ccm.h"
#include "hardware/mx8mn_pinmux.h"

#include <arch/board/board.h>

/****************************************************************************
 * Helper Functions for Board-Specific Timer Configuration
 ****************************************************************************/

/**
 * Initialize timer_io_channels_t structure from Timer and GPIO enums
 *
 * For i.MX8MN, each PWM module has one output channel.
 * The pin muxing is already defined in mx8mn_pinmux.h as IOMUXC_* constants.
 */
static inline constexpr timer_io_channels_t initIOTimerChannel(
	const io_timers_t io_timers_conf[MAX_IO_TIMERS],
	Timer::TimerChannel timer_channel,
	GPIO::GPIOPin pin)
{
	timer_io_channels_t ret{};

	/* Get pin identifier from board-level defines.
	 * Pin values are enum identifiers (PWM_PIN_*) that get mapped
	 * to IOMUXC configurations in the NuttX driver layer.
	 *
	 * This allows board variants to override pin mappings via board.h
	 * without modifying this code.
	 *
	 * Default pins (defined in board.h):
	 *   PWM1: BOARD_PWM1_PIN (default: PWM_PIN_SPDIF_EXT_CLK)
	 *   PWM2: BOARD_PWM2_PIN (default: PWM_PIN_SPDIF_RX)
	 *   PWM3: BOARD_PWM3_PIN (default: PWM_PIN_SPDIF_TX)
	 *   PWM4: BOARD_PWM4_PIN (default: PWM_PIN_SAI3_MCLK)
	 */

	uint32_t pin_id = 0;

	switch (timer_channel.timer) {
	case Timer::PWM1:
#ifdef BOARD_PWM1_PIN
		pin_id = BOARD_PWM1_PIN;
#else
		pin_id = 1;  /* PWM_PIN_SPDIF_EXT_CLK */
#endif
		break;

	case Timer::PWM2:
#ifdef BOARD_PWM2_PIN
		pin_id = BOARD_PWM2_PIN;
#else
		pin_id = 2;  /* PWM_PIN_SPDIF_RX */
#endif
		break;

	case Timer::PWM3:
#ifdef BOARD_PWM3_PIN
		pin_id = BOARD_PWM3_PIN;
#else
		pin_id = 3;  /* PWM_PIN_SPDIF_TX */
#endif
		break;

	case Timer::PWM4:
#ifdef BOARD_PWM4_PIN
		pin_id = BOARD_PWM4_PIN;
#else
		pin_id = 4;  /* PWM_PIN_SAI3_MCLK */
#endif
		break;

	default:
		break;
	}

	ret.gpio_out = pin_id;
	ret.gpio_in = 0;  // PWM input not supported yet

	// i.MX8MN PWM modules have only one channel each
	ret.timer_channel = 1;

	// Find timer index in io_timers_conf array
	ret.timer_index = 0xff;
	const uint32_t timer_base = timerBaseRegister(timer_channel.timer);

	for (int i = 0; i < MAX_IO_TIMERS; ++i) {
		if (io_timers_conf[i].base == timer_base) {
			ret.timer_index = i;
			break;
		}
	}

	constexpr_assert(ret.timer_index != 0xff, "Timer not found");

	return ret;
}

/**
 * Initialize io_timers_t structure from Timer enum
 *
 * Sets up base address, clock gate, and IRQ for each PWM module.
 */
static inline constexpr io_timers_t initIOTimer(Timer::Timer timer)
{
	bool nuttx_config_timer_enabled = false;
	io_timers_t ret{};

	switch (timer) {
	case Timer::PWM1:
		ret.base = MX8M_PWM1;
		ret.clock_register = CCM_PWM1_CLK_GATE;
		ret.clock_bit = 0;  // Unused for i.MX8MN
		ret.vectorno = 0;   // IRQ not used yet
#ifdef CONFIG_MX8MN_PWM1
		nuttx_config_timer_enabled = true;
#endif
		break;

	case Timer::PWM2:
		ret.base = MX8M_PWM2;
		ret.clock_register = CCM_PWM2_CLK_GATE;
		ret.clock_bit = 0;
		ret.vectorno = 0;
#ifdef CONFIG_MX8MN_PWM2
		nuttx_config_timer_enabled = true;
#endif
		break;

	case Timer::PWM3:
		ret.base = MX8M_PWM3;
		ret.clock_register = CCM_PWM3_CLK_GATE;
		ret.clock_bit = 0;
		ret.vectorno = 0;
#ifdef CONFIG_MX8MN_PWM3
		nuttx_config_timer_enabled = true;
#endif
		break;

	case Timer::PWM4:
		ret.base = MX8M_PWM4;
		ret.clock_register = CCM_PWM4_CLK_GATE;
		ret.clock_bit = 0;
		ret.vectorno = 0;
#ifdef CONFIG_MX8MN_PWM4
		nuttx_config_timer_enabled = true;
#endif
		break;

	default:
		break;
	}

	// This is not strictly required, but for consistency let's make sure
	// NuttX PWM timers are disabled when used by PX4
	constexpr_assert(!nuttx_config_timer_enabled,
			 "IO Timer requires NuttX PWM config to be disabled (CONFIG_MX8MN_PWMx)");

	return ret;
}
