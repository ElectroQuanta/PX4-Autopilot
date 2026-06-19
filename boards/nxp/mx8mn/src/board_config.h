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
 * NXP mx8mn-e internal definitions
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

__BEGIN_DECLS

/* These headers provide the i.MX8MN specific register offsets and bit definitions */
#include <hardware/mx8mn_pinmux.h>
#include <hardware/mx8mn_gpio.h>

/* This header contains your specific board pin assignments */
#include <arch/board/board.h>

/* ADC channels - Define dummy values for now */
#define ADC_BATTERY_VOLTAGE_CHANNEL  ((uint8_t)0)  /* Dummy - no actual ADC */
#define ADC_BATTERY_CURRENT_CHANNEL ((uint8_t)0)   /* Dummy - no actual ADC */

__END_DECLS


/** =========================== 12C ============================ */
/* * I2C Pin Definitions
 * Signature: mux_reg, mux_mode, input_reg, input_daisy, config_reg, sion,
 * config
 *
 * SION Bit: Software Input On
 * IMPORTANT: I2C requires the SION bit set so the controller can
 * read back the line state even when it is driving the line.
 * Inputs of I2Cn_SCL and I2Cn_SDA also need to be manually enabled by
 * setting the SION bit in the IOMUX after the corresponding PADs are
 * selected as I2C function.
 */

/* Use the Mux Mode + SION Flag (bit 4) */
#define I2C_MUX_SION 1

/* I2C4 - Sensors */
#define IOMUX_I2C4_SCL IOMUXC_I2C4_SCL_I2C4_SCL, I2C_MUX_SION, I2C_PAD_CTRL
#define IOMUX_I2C4_SDA IOMUXC_I2C4_SDA_I2C4_SDA, I2C_MUX_SION, I2C_PAD_CTRL


/* UART3 + Flow Control (Sacrificing SPI1 Pins): Telem Radio */

#define IOMUX_UART3_RX   IOMUXC_ECSPI1_SCLK_UART3_RX, 1, UART_PAD_CTRL
#define IOMUX_UART3_TX   IOMUXC_ECSPI1_MOSI_UART3_TX, 1, UART_PAD_CTRL
#define IOMUX_UART3_RTS  IOMUXC_ECSPI1_SS0_UART3_RTS_B, 1, UART_PAD_CTRL
#define IOMUX_UART3_CTS  IOMUXC_ECSPI1_MISO_UART3_CTS_B, 1, UART_PAD_CTRL

/* Sensor Power Control */
#define GPIO_VDD_3V3_SENSORS_EN (GPIO_PORT1 | GPIO_PIN9 | GPIO_OUTPUT | GPIO_OUTPUT_ONE)
#define VDD_3V3_SENSORS_EN(v) mx8mn_gpio_write(GPIO_VDD_3V3_SENSORS_EN, !(v))

/* Timer I/O PWM Configuration
 *
 * 4 PWM outputs are configured using the i.MX8MN PWM modules:
 *   PWM1: GPIO1_IO01
 *   PWM2: GPIO1_IO13
 *   PWM3: GPIO1_IO10
 *   PWM4: SAI3_MCLK
 */
#define DIRECT_PWM_OUTPUT_CHANNELS  4

/* Board-specific PWM frequency (Hz) */
#define BOARD_PWM_FREQ              400     /* 400 Hz for standard ESC */
#define BOARD_ONESHOT_FREQ          8000000 /* 8 MHz for OneShot125 */

/* IOMUX for PWM pins */
#define IOMUX_PWM1_OUT  IOMUXC_GPIO1_IO01_PWM1_OUT, 1, PWM_PAD_CTRL
#define IOMUX_PWM2_OUT  IOMUXC_GPIO1_IO13_PWM2_OUT, 5, PWM_PAD_CTRL
#define IOMUX_PWM3_OUT  IOMUXC_GPIO1_IO10_PWM3_OUT, 2, PWM_PAD_CTRL
#define IOMUX_PWM4_OUT  IOMUXC_SAI3_MCLK_PWM4_OUT, 1, PWM_PAD_CTRL


/* High-Resolution Timer (HRT) Configuration **********************************/

/* HRT uses GPT1 (General Purpose Timer 1) for 1 MHz time base
 *
 * Timer:   GPT1 (32-bit free-running counter)
 * Clock:   24 MHz crystal oscillator (OSC_24M_REF_CLK)
 * Freq:    1 MHz (24 MHz / 24 prescaler)
 * Channel: Output Compare Channel 1 for HRT callbacks
 */
#define HRT_TIMER           1    /* Use GPT1 */
#define HRT_TIMER_CHANNEL   1    /* Use Output Compare Channel 1 */

/* Optional: Uncomment to enable PPM input capture on channel 2
 * #define HRT_PPM_CHANNEL  2
 */

/* GPT clock frequency - 24 MHz crystal oscillator
 * Used by High-Resolution Timer (HRT) for 1 MHz time base
 */
#define BOARD_GPT_FREQUENCY  24000000  /* 24 MHz */

/* SPI configuration **********************************************************/
/* SPI1 is defined by default;
 * SPI2 requires (see mx8mn_spidev.c):
 * - IOMUXC: see mx8mn_pinmux.h
 *   - IOMUXC_SPI2_MISO
 *   - IOMUXC_SPI2_MOSI
 *   - IOMUXC_SPI2_SCLK
 *   - IOMUXC_SPI2_CS: choose a pin that provides GPIO (SW control, ALT5)
 * - GPIO: see mx8mn_gpio.h
 *   - GPIO_SPI2_CS
 */

#define IOMUXC_SPI2_MISO IOMUXC_ECSPI2_MISO_ECSPI2_MISO, 0, SPI_PAD_CTRL
#define IOMUXC_SPI2_MOSI IOMUXC_ECSPI2_MOSI_ECSPI2_MOSI, 0, SPI_PAD_CTRL
#define IOMUXC_SPI2_CLK IOMUXC_ECSPI2_SCLK_ECSPI2_SCLK, 0, SPI_PAD_CTRL

/* SPI2 Chip Selects */
#define GPIO_SPI2_CS_BMI088_GYRO  (GPIO_PORT3 | GPIO_PIN24 | GPIO_OUTPUT | GPIO_OUTPUT_ONE)
#define GPIO_SPI2_CS_BMI088_ACCEL (GPIO_PORT3 | GPIO_PIN25 | GPIO_OUTPUT | GPIO_OUTPUT_ONE)
#define GPIO_SPI2_CS_ICM42688     (GPIO_PORT3 | GPIO_PIN21 | GPIO_OUTPUT | GPIO_OUTPUT_ONE)
#define GPIO_SPI2_CS_FLASH       (GPIO_PORT3 | GPIO_PIN19 | GPIO_OUTPUT | GPIO_OUTPUT_ONE)

/* Data Ready (DRDY) Pins as Inputs with Pull-ups */
#define MX8MN_GPIO_DRDY_CONFIG (GPIO_INTERRUPT | GPIO_INTBOTH_EDGES | PAD_CTL_HYS | PAD_CTL_PE | PAD_CTL_PUE)

#define GPIO_ICM42688_DRDY     (GPIO_PORT3 | GPIO_PIN20 | MX8MN_GPIO_DRDY_CONFIG)
#define GPIO_BMI088_ACCEL_DRDY (GPIO_PORT3 | GPIO_PIN22 | MX8MN_GPIO_DRDY_CONFIG)
#define GPIO_BMI088_GYRO_DRDY  (GPIO_PORT3 | GPIO_PIN23 | MX8MN_GPIO_DRDY_CONFIG)
/* This board provides the board_on_reset interface */

#define BOARD_HAS_ON_RESET 1

/* For minimal bring-up we do not configure any GPIOs yet.
 * px4_gpio_init() will simply not be called.
 */

#define PX4_GPIO_INIT_LIST                                                     \
  {                                                                            \
    GPIO_VDD_3V3_SENSORS_EN,                                                   \
  }

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

__BEGIN_DECLS

#ifndef __ASSEMBLY__

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/****************************************************************************
 * Name: mx8mn_i2cdev_initialize
 *
 * Description:
 *   Called to configure all i2c
 *
 ****************************************************************************/

int mx8mn_i2cdev_initialize(void);


/****************************************************************************
 * Name: mx8mn_iomuxc_set_pin_config
 *
 * Description:
 *   Configure the IOMUXC pin configuration.
 *   The first five parameters can be filled with the pin function ID macros.
 *
 ****************************************************************************/

/* void mx8mn_iomuxc_config(uint32_t mux_register, */
/*                          uint32_t mux_mode, */
/*                          uint32_t input_register, */
/*                          uint32_t input_daisy, */
/*                          uint32_t config_register, */
/*                          uint32_t sion, */
/*                          uint32_t config); */

/************************************************************************************
 * Name: mx8mn_spidev_initialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the NXP MX8MN board.
 *
 ************************************************************************************/

void mx8mn_spidev_initialize(void);

/************************************************************************************
 * Name: mx8mn_spi_bus_initialize
 *
 * Description:
 *   Called to configure SPI Buses.
 *
 ************************************************************************************/

int  mx8mn_spi_bus_initialize(void);

/************************************************************************************
 * Name: mx8mn_timer_initialize
 *
 * Description:
 *   Called to initialize the HRT (High-Resolution Timer) subsystem.
 *   This configures GPT1 for 1 MHz operation used by PX4's timing system.
 *
 ************************************************************************************/
void mx8mn_timer_initialize(void);

/****************************************************************************************************
 * Name: board_spi_reset board_peripheral_reset
 *
 * Description:
 *   Called to reset SPI and the perferal bus
 *
 ****************************************************************************************************/
void board_peripheral_reset(int ms);

/************************************************************************************
 * Name: mx8mn_bringup
 *
 * Description:
 *   Bring up board features
 *
 ************************************************************************************/

#if defined(CONFIG_BOARDCTL) || defined(CONFIG_BOARD_INITIALIZE)
int mx8mn_bringup(void);
#endif

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
