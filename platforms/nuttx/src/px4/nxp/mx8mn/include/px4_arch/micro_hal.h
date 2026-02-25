/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
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

#include <nuttx/compiler.h>
#include <nuttx/irq.h>
#include <stdint.h>


#include "../../../nxp_common/include/px4_arch/micro_hal.h"

__BEGIN_DECLS

#ifndef __ASSEMBLY__
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/spi/spi_transfer.h> // For SPI commands to work too
#endif

#include <chip.h>
#include <mx8mn_i2c.h>
#include <mx8mn_gpio.h>
#include <mx8mn_ecspi.h>

#define PX4_NUMBER_I2C_BUSES   2
#define PX4_NUMBER_SPI_BUSES 2

#define GPIO_OUTPUT_SET             GPIO_OUTPUT_ONE
#define GPIO_OUTPUT_CLEAR GPIO_OUTPUT_ZERO

/** ================== PX4 stubs =============================== */
/* PX4 expects these lengths to exist at compile time */
#define PX4_CPU_UUID_BYTE_LENGTH       16
#define PX4_CPU_UUID_WORD32_LENGTH     (PX4_CPU_UUID_BYTE_LENGTH / 4)

#define PX4_CPU_MFGUID_BYTE_LENGTH     16
#define PX4_CPU_MFGUID_WORD32_LENGTH   (PX4_CPU_MFGUID_BYTE_LENGTH / 4)

/* Board HW type string used by src/lib/version */
__EXPORT const char *board_get_hw_type_name(void);

/* CPU IDs (you can keep them zero for now) */
__EXPORT void px4_cpu_uuid_get(uint32_t *uuid_words);
__EXPORT void px4_cpu_mfguid_get(uint32_t *mfguid_words);
/** ============================================================ */


/* bus_num is 1-based on mx8mn such as PX4, so no adjustment is required */
#define PX4_BUS_OFFSET       0  /* mx8mn buses are 1-based */

/* Map the initialization function directly */
#define px4_i2cbus_initialize(bus_num)  mx8mn_i2cbus_initialize(bus_num)
#define px4_i2cbus_uninitialize(pdev)   mx8mn_i2cbus_uninitialize(pdev)

/* Do the same for SPI if needed */
#define px4_spibus_initialize(bus_num)  mx8mn_spibus_initialize(bus_num)


#define px4_arch_configgpio(pinset)             mx8mn_gpio_config(pinset)
#define px4_arch_unconfiggpio(pinset)
#define px4_arch_gpioread(pinset)               mx8mn_gpio_read(pinset)
#define px4_arch_gpiowrite(pinset, value) mx8mn_gpio_write(pinset, value)

/* Use static inline to allow the compiler to optimize out the function call 
   while maintaining type safety and avoiding "redefinition" errors. */

/* 1. Define the actual functions as static inlines for type safety */
/* static inline void px4_arch_configgpio(uint32_t pinset) { */
/*     (void)mx8mn_gpio_config((gpio_pinset_t)pinset); */
/* } */

/* static inline void px4_arch_gpiowrite(uint32_t pinset, bool value) { */
/*     (void)mx8mn_gpio_write((gpio_pinset_t)pinset, value); */
/* } */

/* static inline bool px4_arch_gpioread(uint32_t pinset) { */
/*     return mx8mn_gpio_read((gpio_pinset_t)pinset); */
/* } */

/* static inline void px4_arch_unconfiggpio(uint32_t pinset) { */
/*     /\* Nothing required for i.MX8MN *\/ */
/* } */

/* Mask to clear Mode, Value, and Interrupt bits (31, 30, 29, 27, 26, 25) */
#define _GPIO_CFG_MASK                                                         \
  (GPIO_MODE_MASK | GPIO_OUTPUT_ONE | GPIO_INTCFG_MASK | GPIO_INTBOTHCFG_MASK)

/* Base macro to clear functional bits and apply new ones */
#define _PX4_MAKE_GPIO(pin_cfg, io) \
    (((uint32_t)(pin_cfg) & ~(_GPIO_CFG_MASK)) | (uint32_t)(io))

/* 1. Configure as Input */
#define PX4_MAKE_GPIO_INPUT(gpio) \
    _PX4_MAKE_GPIO(gpio, GPIO_INPUT)

/* 2. Configure as External Interrupt (Both Edges) */
#define PX4_MAKE_GPIO_EXTI(gpio) \
    _PX4_MAKE_GPIO(gpio, GPIO_INTERRUPT | GPIO_INTBOTH_EDGES)

/* 3. Configure as Output and set HIGH (Initial Value 1) */
#define PX4_MAKE_GPIO_OUTPUT_SET(gpio) \
    _PX4_MAKE_GPIO(gpio, GPIO_OUTPUT | GPIO_OUTPUT_ONE)

/* 4. Configure as Output and set LOW (Initial Value 0) */
#define PX4_MAKE_GPIO_OUTPUT_CLEAR(gpio) \
    _PX4_MAKE_GPIO(gpio, GPIO_OUTPUT | GPIO_OUTPUT_ZERO)

/** ================== TODO =========================== */
/* ADD GPIO Set Event for interrupt handling at PX4 Level
 * - requires IO pins
 */
/* mx8mn_gpiosetevent is added at PX4 level */
int mx8mn_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge, bool event, xcpt_t func, void *arg);

#define px4_arch_gpiosetevent(pinset,r,f,e,fp,a)  mx8mn_gpiosetevent(pinset,r,f,e,fp,a)
/** =================================================== */

__END_DECLS
