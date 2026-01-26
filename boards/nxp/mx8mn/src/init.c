/****************************************************************************
 *
 *   Copyright (c) 2016, 2018 PX4 Development Team. All rights reserved.
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
 * @file init.c
 *
 * NXP fmuk66-e specific early startup code.  This file implements the
 * board_app_initialize() function that is called early by nsh during startup.
 *
 * Code here is run before the rcS script is invoked; it should start required
 * subsystems and perform board-specific initialization.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <px4_platform_common/px4_config.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include <px4_platform_common/init.h>

/****************************************************************************
 * Optional LED functions (only if you later enable LED driver)
 ****************************************************************************/
__BEGIN_DECLS
void led_init(void);
void led_on(int led);
void led_off(int led);
__END_DECLS

/****************************************************************************
 * board_on_reset
 ****************************************************************************/

void board_on_reset(int status)
{
    /* For now: nothing special.
     * Later you can ensure ESC outputs are safe here.
     */
    (void)status;
}

/****************************************************************************
 * board_read_VBUS_state
 *
 * Returns 0 if USB VBUS is present, 1 otherwise.
 * For now, just report "not connected".
 ****************************************************************************/

int board_read_VBUS_state(void)
{
    /* TODO: hook to real GPIO/ADC later */
    return 1;
}

/****************************************************************************
 * Name: mx8mn_boardinitialize
 *
 * Called very early (before apps) by NuttX.
 ****************************************************************************/

__EXPORT void mx8mn_board_initialize(void)
{
    board_on_reset(-1);

    /* For minimal bring-up we do NOT:
     *  - configure GPIOs
     *  - configure timers
     *  - configure sensors, SD, etc.
     */
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * PX4 entry: do platform-level init here.
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
    (void)arg;

    /* PX4 core platform initialization
     * (work queues, uORB, etc.)
     */
    px4_platform_init();

    return OK;
}

