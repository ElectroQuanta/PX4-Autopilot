/****************************************************************************
 * boards/nxp/mx8mn/src/init.c
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
#include <mx8mn_rptun.h>

#include <px4_arch/io_timer.h> // io_timer_channel_get_as_pwm_input
#include <systemlib/px4_macros.h> // arraySize

#include <nuttx/config.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include <nuttx/semaphore.h>
#include <nuttx/clock.h>
#include <nuttx/kthread.h>
#include <time.h>

#ifdef CONFIG_RPTUN
#include <nuttx/rptun/rptun.h>
#include <nuttx/rptun/openamp.h>
#endif

#ifdef CONFIG_MX8MN_RPMSG
#include "mx8mn_rptun.h"
#endif

#include <px4_platform_common/module.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define RPMSGFS_WAIT_SECONDS 10

/****************************************************************************
 * Private Data
 ****************************************************************************/

#ifdef CONFIG_MX8MN_RPMSG
static sem_t g_fs_ready_sem;
static bool  g_rpmsg_link_ok = false;
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_MX8MN_RPMSG
static int copy_file(const char *src, const char *dst)
{
  int sfd = open(src, O_RDONLY);
  if (sfd < 0) return -errno;

  int dfd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (dfd < 0)
    {
      close(sfd);
      return -errno;
    }

  char *buf = (char *)malloc(512);
  if (buf == NULL)
    {
      close(sfd);
      close(dfd);
      return -ENOMEM;
    }

  ssize_t n;
  int ret = OK;
  while ((n = read(sfd, buf, 512)) > 0)
    {
      if (write(dfd, buf, n) != n)
        {
          ret = -EIO;
          break;
        }
    }

  free(buf);
  close(sfd);
  close(dfd);
  return ret;
}

static int mount_linux_task(int argc, char *argv[])
{
  int ret;
  int retry = 10;

  mkdir("/mnt", 0777);
  mkdir("/mnt/linux", 0777);
  sleep(5);

  while (retry > 0)
    {
      ret = nx_mount(NULL, "/mnt/linux", "rpmsgfs", 0, "cpu=linux");
      if (ret == OK)
        {
          struct stat st;
          if (stat("/mnt/linux/mtd_params", &st) == OK)
            {
              syslog(LOG_INFO, "[FS]: Linux storage linked and verified at /mnt/linux\n");

              const char *files[] = {"mtd_params", "mtd_caldata", NULL};
              for (int i = 0; files[i] != NULL; i++)
                {
                  char src[64], dst[64];
                  snprintf(src, sizeof(src), "/mnt/linux/%s", files[i]);
                  snprintf(dst, sizeof(dst), "/fs/%s", files[i]);

                  copy_file(src, dst);
                }

              g_rpmsg_link_ok = true;
              break;
            }
          else
            {
              umount("/mnt/linux");
            }
        }
      usleep(500000);
      retry--;
    }

  nxsem_post(&g_fs_ready_sem);
  return 0;
}
#endif

/****************************************************************************
 * Optional LED functions
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
  for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
    px4_arch_configgpio(PX4_MAKE_GPIO_INPUT(io_timer_channel_get_as_pwm_input(i)));
  }
  if (status >= 0) {
    up_mdelay(100);
  }
}

int board_read_VBUS_state(void)
{
    return 1;
}

/****************************************************************************
 * Name: mx8mn_board_initialize
 ****************************************************************************/

__EXPORT void mx8mn_board_initialize(void)
{
  board_on_reset(-1);

  /* Configure IOMUX for VDD_3V3_SENSORS_EN (GPIO1_IO09)*/

  mx8mn_iomuxc_config( IOMUX_GPIO109 );


  /* Configure I2C4 Pins (SCL, SDA) with SION enabled */
  mx8mn_iomuxc_config(IOMUX_I2C4_SCL);
  mx8mn_iomuxc_config(IOMUX_I2C4_SDA);

  const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
  px4_gpio_init(gpio, arraySize(gpio));
}

#define PARAM_MTD_SIZE (64 * 1024)

#ifdef CONFIG_MX8MN_RPMSG
int param_sync_thread(int argc, char *argv[]);
#endif

/****************************************************************************
 * Name: board_app_initialize
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
  (void)arg;
  int ret;

  volatile int dbg = 1;
  while (dbg)
    ;

  /* syslog(LOG_INFO, "[VDD_3V3] Before disable (GPIO1_9): %d\n", mx8mn_gpio_read(GPIO_VDD_3V3_SENSORS_EN)); */
  VDD_3V3_SENSORS_EN(true);
  /* syslog(LOG_INFO, "[VDD_3V3] After disable (GPIO1_9): %d\n", mx8mn_gpio_read(GPIO_VDD_3V3_SENSORS_EN)); */
  
  /* syslog(LOG_INFO, "[VDD_3V3] Before enable (GPIO1_9): %d\n", mx8mn_gpio_read(GPIO_VDD_3V3_SENSORS_EN)); */
  /* VDD_3V3_SENSORS_EN(true); */
  /* syslog(LOG_INFO, "[VDD_3V3] After enable (GPIO1_9): %d\n", mx8mn_gpio_read(GPIO_VDD_3V3_SENSORS_EN)); */

#ifdef CONFIG_MX8MN_RPMSG
  nxsem_init(&g_fs_ready_sem, 0, 0);
  syslog(LOG_INFO, "[RPTUN]: Starting RPTUN...\n");
  mx8mn_rptun_init("imx8mn-shmem", "linux");
  kthread_create("linux_link", SCHED_PRIORITY_DEFAULT, 2048, mount_linux_task, NULL);
#endif

  mkdir("/fs", 0777);
  ret = nx_mount(NULL, "/fs", "tmpfs", 0, "mode=0777");
  
  if (ret == OK)
    {
#ifdef CONFIG_MX8MN_RPMSG
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec += RPMSGFS_WAIT_SECONDS;

      ret = nxsem_timedwait_uninterruptible(&g_fs_ready_sem, &ts);
#endif

      mkdir("/fs/microsd", 0777);
      mkdir("/fs/microsd/log", 0777);
    }

  /* configure SPI interfaces */
  mx8mn_spidev_initialize();

#ifdef CONFIG_SPI
  ret = mx8mn_spi_bus_initialize();
#endif

  /* PX4 core init */
  px4_platform_init();

#ifdef CONFIG_MX8MN_RPMSG
  kthread_create("param_sync", 30, 2048, param_sync_thread, NULL);
#endif

  return OK;
}
