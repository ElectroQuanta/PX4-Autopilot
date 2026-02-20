/****************************************************************************
 * configs/nxp/mx8mn/include/board.h
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __CONFIG_NXP_MX8MN_INCLUDE_BOARD_H
#define __CONFIG_NXP_MX8MN_INCLUDE_BOARD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
#  include <stdint.h>
#  include <stdbool.h>
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Console UART IOMUX configuration */

#define IOMUX_CONSOLE_UART_RX IOMUXC_UART4_RXD_UART4_RX, 0, UART_PAD_CTRL
#define IOMUX_CONSOLE_UART_TX IOMUXC_UART4_TXD_UART4_TX, 0, UART_PAD_CTRL

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

/* PWM Pin Configuration ****************************************************/
/* PWM outputs for motor control (ESC). These can be overridden by defining
 * them before including this file, allowing board variants to use different
 * pins without modifying the driver code.
 *
 * Default configuration uses SPDIF and SAI3 pins, leaving GPIO1 free for
 * expansion. To use GPIO1 pins instead, define these before including board.h:
 *
 *   #define BOARD_PWM1_PIN  IOMUXC_GPIO1_IO00_PWM1_OUT
 *   #define BOARD_PWM2_PIN  IOMUXC_GPIO1_IO01_PWM2_OUT
 *   #define BOARD_PWM3_PIN  IOMUXC_GPIO1_IO02_PWM3_OUT
 *   #define BOARD_PWM4_PIN  IOMUXC_GPIO1_IO03_PWM4_OUT
 */

#ifndef BOARD_PWM1_PIN
#  define BOARD_PWM1_PIN  IOMUXC_SPDIF_EXT_CLK_PWM1_OUT
#endif

#ifndef BOARD_PWM2_PIN
#  define BOARD_PWM2_PIN  IOMUXC_SPDIF_RX_PWM2_OUT
#endif

#ifndef BOARD_PWM3_PIN
#  define BOARD_PWM3_PIN  IOMUXC_SPDIF_TX_PWM3_OUT
#endif

#ifndef BOARD_PWM4_PIN
#  define BOARD_PWM4_PIN  IOMUXC_SAI3_MCLK_PWM4_OUT
#endif

/* LED definitions **********************************************************/

/* LED index values for use with board_userled() */

#define BOARD_LED_1       0
#define BOARD_LED_2       1
#define BOARD_LED_3       2
#define BOARD_LED_4       3
#define BOARD_NLEDS       4

/* LED bits for use with board_userled_all() */

#define BOARD_LED_1_BIT   (1 << BOARD_LED_1)
#define BOARD_LED_2_BIT   (1 << BOARD_LED_2)
#define BOARD_LED_3_BIT   (1 << BOARD_LED_3)
#define BOARD_LED_4_BIT   (1 << BOARD_LED_4)

/* If CONFIG_ARCH_LEDs is defined, then NuttX will control the LED on board.
 * The following definitions describe how NuttX controls
 * the LEDs:
 *
 *   SYMBOL                Meaning                      LED
 *   -------------------  ----------------------------  --------------------
 */

#define LED_STARTED       0 /* NuttX has been started    None  */
#define LED_HEAPALLOCATE  1 /* Heap has been allocated   ON(1),  OFF(2) */
#define LED_IRQSENABLED   2 /* Interrupts enabled        OFF(1), ON(2)  */
#define LED_STACKCREATED  3 /* Idle stack created        ON(1), ON(2) */
#define LED_INIRQ         4 /* In an interrupt          (no change) */
#define LED_SIGNAL        5 /* In a signal handler      (no change) */
#define LED_ASSERTION     6 /* An assertion failed       ON(3) */
#define LED_PANIC         7 /* The system has crashed    FLASH(1,2) */
#define LED_IDLE          8 /* idle loop                 FLASH(4) */

/* Button definitions *******************************************************/

/* The Verdin board has four switch buttons and four on/off buttons.
 * They are not connected to the board by default: it is up to the user to
 * connect them to one of the multiple available headers pins.
 * Here we choose (arbitrary) to connect SW11 to GPIO_5_CSI header pin
 *
 * 1. SW11    GPIO1_7 (= GPIO_5_CSI)
 */

#define BUTTON_1          0
#define NUM_BUTTONS       1

#define BUTTON_1_BIT      (1 << BUTTON_1)

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/


/************************************************************************************
 * Name: mx8mn_boardinitialize
 *
 * Description:
 *   All MX8MN architectures must provide the following entry point.  This entry point
 *   is called early in the initialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

void mx8mn_boardinitialize(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif  /* __CONFIG_NXP_MX8MN_INCLUDE_BOARD_H */
