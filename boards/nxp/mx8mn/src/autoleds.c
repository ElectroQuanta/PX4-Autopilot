/****************************************************************************
 *
 *   Copyright (C) 2016-2018 Gregory Nutt. All rights reserved.
 *   Authors: Gregory Nutt <gnutt@nuttx.org>
 *            David Sidrane <david_s5@nscdg.com>
 *
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
 * 3. Neither the name NuttX nor the names of its contributors may be
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
 * This module shall be used during board bring up of Nuttx.
 *
 * The NXP FMUK66-E has a separate Red, Green and Blue LEDs driven by the K66
 * as follows:
 *
 *   LED    K66
 *   ------ -------------------------------------------------------
 *   RED    FB_CS0_b/ UART2_CTS_b / ADC0_SE5b / SPI0_SCK / FTM3_CH1/ PTD1
 *   GREEN  FTM2_FLT0/ CMP0_IN3/ FB_AD6 / I2S0_RX_BCLK/ FTM3_CH5/ ADC1_SE5b/ PTC9
 *   BLUE   CMP0_IN2/ FB_AD7 / I2S0_MCLK/ FTM3_CH4/ ADC1_SE4b/ PTC8
 *
 * If CONFIG_ARCH_LEDs is defined, then NuttX will control the LED on board
 * the NXP fmuk66-e.  The following definitions describe how NuttX controls
 * the LEDs:
 *
 *   SYMBOL                Meaning                 LED state
 *                                                 RED   GREEN  BLUE
 *   -------------------  -----------------------  -----------------
 *   LED_STARTED          NuttX has been started    OFF  OFF  OFF
 *   LED_HEAPALLOCATE     Heap has been allocated   OFF  OFF  ON
 *   LED_IRQSENABLED      Interrupts enabled        OFF  OFF  ON
 *   LED_STACKCREATED     Idle stack created        OFF  ON   OFF
 *   LED_INIRQ            In an interrupt          (no change)
 *   LED_SIGNAL           In a signal handler      (no change)
 *   LED_ASSERTION        An assertion failed      (no change)
 *   LED_PANIC            The system has crashed    FLASH OFF OFF
 *   LED_IDLE             K66 is in sleep mode     (Optional, not used)
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

// boards/nxp/mx8mn/src/autoled.c
#include <nuttx/board.h>

void board_autoled_initialize(void) { }
void board_autoled_on(int led)      { (void)led; }
void board_autoled_off(int led)     { (void)led; }
