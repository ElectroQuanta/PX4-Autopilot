/****************************************************************************
 *
 *   Copyright (c) 2013-2018 PX4 Development Team. All rights reserved.
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
 * @file board_config.h
 *
 * NXP fmuk66-e internal definitions
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/board_common.h>
#include <nuttx/compiler.h>
#include <stdint.h>

__BEGIN_DECLS

/* Only pull the generic board header.
 * DO NOT include kinetis.h or kinetis_pinmux.h here.
 */
#include <arch/board/board.h>

/* ADC channels - Define dummy values for now */
#define ADC_BATTERY_VOLTAGE_CHANNEL  ((uint8_t)0)  /* Dummy - no actual ADC */
#define ADC_BATTERY_CURRENT_CHANNEL ((uint8_t)0)   /* Dummy - no actual ADC */

/* SRF05 on iMX8MN - adjust pin numbers based on your schematic */
// defined in ./platforms/nuttx/NuttX/nuttx/arch/arm/src/kinetis/kinetis.h
/* #define GPIO_ULTRASOUND_TRIGGER  /\* PTD0 *\/  (GPIO_LOWDRIVE | GPIO_OUTPUT_ZERO | PIN_PORTD | PIN0) */
/* #define GPIO_ULTRASOUND_ECHO     /\* PTA10 *\/ (GPIO_PULLUP | PIN_INT_BOTH | PIN_PORTA | PIN10) */

/* cache-aligned allocation used by uORB and others */
__EXPORT void *px4_cache_aligned_alloc(size_t size);
__EXPORT void  px4_cache_aligned_free(void *ptr);

__END_DECLS

/* For minimal bring-up we do not configure any GPIOs yet.
 * px4_gpio_init() will simply not be called.
 */

#define PX4_GPIO_INIT_LIST                                                     \
  {}

/* PX4 uses 2x 32-bit words from the CPU UUID as a 64-bit unique ID (MAVLink UID).
 * For bring-up we just point it at words 0 and 1.
 */
#ifndef PX4_CPU_UUID_WORD32_UNIQUE_H
#  define PX4_CPU_UUID_WORD32_UNIQUE_H 0
#endif

#ifndef PX4_CPU_UUID_WORD32_UNIQUE_M
#  define PX4_CPU_UUID_WORD32_UNIQUE_M 1
#endif

#if (PX4_CPU_UUID_WORD32_UNIQUE_H >= PX4_CPU_UUID_WORD32_LENGTH) || \
    (PX4_CPU_UUID_WORD32_UNIQUE_M >= PX4_CPU_UUID_WORD32_LENGTH)
#  error "PX4_CPU_UUID_WORD32_UNIQUE_* out of range"
#endif



/* No timers, ADCs, CAN, SD, etc. defined at this stage. */
