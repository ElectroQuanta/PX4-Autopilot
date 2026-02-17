/****************************************************************************
 *
 *   Copyright (C) 2020 PX4 Development Team. All rights reserved.
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

#include <px4_platform_common/px4_config.h>
#include <systemlib/px4_macros.h>
#include <arch/board/board.h>
#include <errno.h>

#include <mx8mn_gpio.h> 

/**************************************************************************
 * Name: mx8mn_gpiosetevent
 *
 * Description:
 * Sets/clears GPIO based event and interrupt triggers for i.MX8MN.
 *
 *************************************************************************/
int mx8mn_gpiosetevent(uint32_t pinset, bool risingedge, bool fallingedge,
                       bool event, xcpt_t func, void *arg)
{
    int ret = -ENOSYS;

    /* 1. Disable Interrupts / Detach Logic */
    if (func == NULL) {
        /* Disable the interrupt on this pin */
        mx8mn_gpio_irq_disable(pinset);
        /* Detach the callback (pass NULL) */
        ret = mx8mn_gpio_irq_attach(pinset, NULL, NULL);
        return ret;
    }

    /* 2. Configure the Edge Detection Bits 
     * We need to modify the 'pinset' variable to include the correct
     * hardware bits for the requested edges.
     */
    
    // Clear existing interrupt config bits (INTCFG and INTBOTHCFG)
    pinset &= ~(GPIO_INTCFG_MASK | GPIO_INTBOTHCFG_MASK);

    // Set new bits based on arguments
    if (risingedge && fallingedge) {
        pinset |= GPIO_INTBOTH_EDGES;
    } else if (risingedge) {
        pinset |= GPIO_INT_RISING_EDGE;
    } else if (fallingedge) {
        pinset |= GPIO_INT_FALLING_EDGE;
    } else {
        /* Default to Low Level if no edge specified (safety) */
        pinset |= GPIO_INT_LOW_LEVEL;
    }

    /* 3. Apply the Configuration to Hardware
     * This updates the ICR (Interrupt Config Register) in the GPIO block.
     * Without this, the hardware won't know which edge to listen for.
     */
    ret = mx8mn_gpio_config(pinset);
    if (ret < 0) {
        return ret;
    }

    /* 4. Attach the Callback */
    ret = mx8mn_gpio_irq_attach(pinset, func, arg);
    if (ret < 0) {
        return ret;
    }

    /* 5. Enable the Interrupt (IMR Register) */
    mx8mn_gpio_irq_enable(pinset);

    return OK;
}
