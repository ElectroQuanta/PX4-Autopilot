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
 * NXP mx8mn specific early startup code.  This file implements the
 * board_app_initialize() function that is called early by nsh during startup.
 *
 * Code here is run before the rcS script is invoked; it should start required
 * subsystems and perform board-specific initialization.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <px4_platform_common/px4_config.h>
#include <px4_platform/gpio.h> // px4_gpio_init

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include <px4_platform_common/init.h>

#include "board_config.h"
#include <nuttx/spi/spi.h>
#include <nuttx/i2c/i2c_master.h>

#include <mx8mn_iomuxc.h>

#include <px4_arch/io_timer.h> // io_timer_channel_get_as_pwm_input
#include <systemlib/px4_macros.h> // arraySize
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
  /*
   * Reconfigure PWM output pins as inputs (Hi-Z) to ensure ESCs
   * see no signal and disarm before the reset completes.
   */
  for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
    px4_arch_configgpio(
        PX4_MAKE_GPIO_INPUT(io_timer_channel_get_as_pwm_input(i))
        );
  }

  /*
   * Give ESCs time to recognize the missing signal and disarm.
   * Only needed on firmware-initiated resets, not bootloader resets.
   */
  if (status >= 0) {
    up_mdelay(100);
  }
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

    /* Configure HRT (High-Resolution Timer) - GPT1 at 1 MHz
	 * (called by px4_platform_init() )
	 */
    /* mx8mn_timer_initialize(); */

    /* For minimal bring-up we do NOT yet:
     *  - configure LEDs
     *  - configure GPIOs
     *  - configure sensors, SD, etc.
     */

    /* /\* configure LEDs *\/ */
    /* board_autoled_initialize(); */

    /* const uint32_t gpio[] = PX4_GPIO_INIT_LIST; */
    /* px4_gpio_init(gpio, arraySize(gpio)); */

    /* /\* Power on Spektrum *\/ */

    /* VDD_3V3_SPEKTRUM_POWER_EN(true); */
}

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#define PARAM_MTD_SIZE (64 * 1024) // 64KB for parameters

/****************************************************************************
 * Name: board_i2c_init
 *
 * Used to debug I2C initialization
 ****************************************************************************/

/* static void board_i2c_init(void) { */

/*   /\* 1. Hardware Pin Muxing *\/ */
/*   mx8mn_iomuxc_config(IOMUX_I2C1_SCL); */
/*   mx8mn_iomuxc_config(IOMUX_I2C1_SDA); */

/*   mx8mn_iomuxc_config(IOMUX_I2C2_SCL); */
/*   mx8mn_iomuxc_config(IOMUX_I2C2_SDA); */

/*   mx8mn_iomuxc_config(IOMUX_I2C3_SCL); */
/*   mx8mn_iomuxc_config(IOMUX_I2C3_SDA); */
    
/*   mx8mn_iomuxc_config(IOMUX_I2C4_SCL); */
/*   mx8mn_iomuxc_config(IOMUX_I2C4_SDA); */

/*   /\* Test all available buses *\/   */
/* for (int i = 0; i < PX4_NUMBER_I2C_BUSES + 1; i++) { */
/*     struct i2c_master_s *test_ptr = mx8mn_i2cbus_initialize(i); */
/*     if (test_ptr != NULL) { */
/*         syslog(LOG_INFO, "[I2C] Phys Idx %d VALID\r\n", i); */
        
/*         // Try to read the WHO_AM_I register (0x00) of the IST8310 (0x0E) */
/*         uint8_t reg = 0x00; */
/*         uint8_t val = 0; */
/*         struct i2c_msg_s msg[2]; */
/*         msg[0].addr = 0x0E; msg[0].flags = 0;          msg[0].buffer = &reg; msg[0].length = 1; */
/*         msg[1].addr = 0x0E; msg[1].flags = I2C_M_READ; msg[1].buffer = &val; msg[1].length = 1; */

/*         if (I2C_TRANSFER(test_ptr, msg, 2) == OK) { */
/*              syslog(LOG_INFO, "[I2C] -> FOUND IST8310 on Phys Idx %d!\r\n", i); */
/*         } */
/*     } */
/*  } */
/* } */

/****************************************************************************
 * Name: board_app_initialize
 *
 * PX4 entry: do platform-level init here.
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
  (void)arg;
  int ret;

  /* Create /fs directory */
  mkdir("/fs", 0777);
    
  /* Mount TMPFS to /fs */
  ret = mount(NULL, "/fs", "tmpfs", 0, "mode=0777");
  if (ret < 0) {
    syslog(LOG_ERR, "[TMPFS]: Failed to mount /fs: errno=%d\n", errno);
  } else {
    syslog(LOG_INFO, "[TMPFS]: /fs mounted OK - PX4 will create param files\n");
  }


  /* configure SPI interfaces */
  
  mx8mn_spidev_initialize();

  /* PX4 core init */

  px4_platform_init();

  /* I2C init (debug)*/

  /* board_i2c_init(); */


#ifdef CONFIG_SPI
	ret = mx8mn_spi_bus_initialize();

	syslog(LOG_INFO, "[SPI]: Bus init = %d\n", ret);

#endif

  return OK;
}

