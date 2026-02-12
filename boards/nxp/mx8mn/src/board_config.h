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

/* /\* cache-aligned allocation used by uORB and others *\/ */
/* __EXPORT void *px4_cache_aligned_alloc(size_t size); */
/* __EXPORT void  px4_cache_aligned_free(void *ptr); */

__END_DECLS


/** =========================== 12C ============================ */
/* I2C Pad Control:
 * ODE (Bit 5) = 1 : Open Drain Enable
 * PUE (Bit 6) = 1 : Pull Up Enable
 * PE  (Bit 8) = 1 : Pull Select Enable
 * HYS (Bit 7) = 1 : Schmitt Trigger Enable
 * DSE (Bits 1-2) = 3 : Max Drive Strength (x6)
 */
#define I2C_PAD_CTRL  (PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE | PAD_CTL_HYS | PAD_CTL_DSE6)

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

/* I2C1 - Often used for internal sensors (Mag/Baro) */
#define IOMUX_I2C1_SCL IOMUXC_I2C1_SCL_I2C1_SCL, I2C_MUX_SION, I2C_PAD_CTRL
#define IOMUX_I2C1_SDA IOMUXC_I2C1_SDA_I2C1_SDA, I2C_MUX_SION, I2C_PAD_CTRL

/* I2C2 - Often used for GPS/External Mag */
#define IOMUX_I2C2_SCL IOMUXC_I2C2_SCL_I2C2_SCL, I2C_MUX_SION, I2C_PAD_CTRL
#define IOMUX_I2C2_SDA IOMUXC_I2C2_SDA_I2C2_SDA, I2C_MUX_SION, I2C_PAD_CTRL

/* I2C2 - Often used for GPS/External Mag */
#define IOMUX_I2C3_SCL IOMUXC_I2C3_SCL_I2C3_SCL, I2C_MUX_SION, I2C_PAD_CTRL
#define IOMUX_I2C3_SDA IOMUXC_I2C3_SDA_I2C3_SDA, I2C_MUX_SION, I2C_PAD_CTRL

/* I2C4 - Common for Power Monitors (INA219) */
#define IOMUX_I2C4_SCL IOMUXC_I2C4_SCL_I2C4_SCL, I2C_MUX_SION, I2C_PAD_CTRL
#define IOMUX_I2C4_SDA IOMUXC_I2C4_SDA_I2C4_SDA, I2C_MUX_SION, I2C_PAD_CTRL


/* /\* I2C1 (PMIC) */
/*  * */
/*  * This device can be pinned out to be either or */

/*  * Bit   Pin Device   Signal Usage                 Conn */
/*  * ----- --- -------  --------------------------- ------ */
/*  * PTB2   83 I2C0_SCL U_ECH Ultrasonic            P13-3 */
/*  * PTB3   84 I2C0_SDA U_TRI Ultrasonic            P13-2 */
/*  * ----- --- -------  --------------------------- ------ */
/*  * */
/*  * Bit   Pin Device   Signal Usage                 Conn */
/*  * ----- --- -------  --------------------------- ------ */
/*  * PTE24  45 I2C0_SCL IIC_SCL NFC Connector, IIC  P2-2 */
/*  * PTE25  46 I2C0_SDA IIC_SDA NFC Connector, IIC  P2-3 */
/*  * ----- --- -------  --------------------------- ------ */
/*  *\/ */

/* #define PIN_I2C1_SCL     PIN_I2C1_SCL_4   /\* PTE24  IIC_SCL *\/ */
/* #define PIN_I2C1_SDA     PIN_I2C1_SDA_4   /\* PTE25  IIC_SDA *\/ */

/* /\* I2C2 (Sensors) */
/*  * */
/*  * Bit   Pin Device   Signal         Usage         Conn */
/*  * ----- --- -------  -------------- ------------- ------ */
/*  * PTC10 115 I2C1_SCL P_SCL, GPS_SCL Pressure, GPS P3-4 */
/*  * PTC11 116 I2C1_SDA P_SDA, GPS_SDA Pressure, GPS P3-5 */
/*  * ----- --- -------  -------------- ------------- ------ */
/*  *\/ */

/* #define PIN_I2C1_SCL     PIN_I2C1_SCL_1   /\* PTC10 GPS / Pressure Sensor*\/ */
/* #define PIN_I2C1_SDA     PIN_I2C1_SDA_1   /\* PTC11 GPS / Pressure Sensor *\/ */
/** ============================================================ */


/** =========================== SPI ============================ */

/* /\* SPI0 FRAM *\/ */

/* #define PIN_SPI0_PCS0    PIN_SPI0_PCS2_1  /\* PTC2 SPI_CS  FRAM_CS   *\/ */
/* #define PIN_SPI0_SCK     PIN_SPI0_SCK_2   /\* PTC5 SPI_CLK FRAM_SCK  *\/ */
/* #define PIN_SPI0_OUT     PIN_SPI0_SOUT_2  /\* PTC6 SPI_OUT FRAM_MOSI *\/ */
/* #define PIN_SPI0_SIN     PIN_SPI0_SIN_2   /\* PTC7 SPI_IN  FRAM_MISO *\/ */

/* /\* SPI1 */
/*  * FXOS8700CQ Accelerometer */
/*  * FXAS21002CQ Gyroscope */
/*  *\/ */

/* #define PIN_SPI1_PCS0    PIN_SPI1_PCS0_1  /\* PTB10 A_CS   *\/ */
/* #define PIN_SPI1_PCS1    PIN_SPI1_PCS1_1  /\* PTB9  GM_CS  *\/ */
/* #define PIN_SPI1_SCK     PIN_SPI1_SCK_1   /\* PTB11 A_SCLK *\/ */
/* #define PIN_SPI1_OUT     PIN_SPI1_SOUT_1  /\* PTB16 A_MOSI *\/ */
/* #define PIN_SPI1_SIN     PIN_SPI1_SIN_1   /\* PTB17 A_MISO *\/ */

/* /\* SPI2 */
/*  * Bit   Pin Device   Signal     Conn */
/*  * ----- --- -------  --------- ------ */
/*  * PTB20 99  SPI2_PCS0 SPI2_CS  P18-5 */
/*  * PTB21 100 SPI2_SCK  SPI2_CLK P18-2 */
/*  * PTB22 101 SPI2_SOUT SPI2_OUT P18-3 */
/*  * PTB23 102 SPI2_SIN SPI2_IN   P18-4 */
/*  * */
/*  *\/ */

/* #define PIN_SPI2_PCS0    PIN_SPI2_PCS0_1  /\* PTB20 SPI2_CS  *\/ */
/* #define PIN_SPI2_SCK     PIN_SPI2_SCK_1   /\* PTB21 SPI2_CLK *\/ */
/* #define PIN_SPI2_OUT     PIN_SPI2_SOUT_1  /\* PTB22 SPI2_OUT *\/ */
/* #define PIN_SPI2_SIN     PIN_SPI2_SIN_1   /\* PTB23 SPI2_IN  *\/ */
/** ============================================================ */

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


/************************************************************************************
 * Name: fmuk66_bringup
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
