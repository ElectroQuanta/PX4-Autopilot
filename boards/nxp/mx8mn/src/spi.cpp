/************************************************************************************
 *
 *   Copyright (C) 2016, 2018 Gregory Nutt. All rights reserved.
 *   Authors: Gregory Nutt <gnutt@nuttx.org>
 *            David Sidrane <david_s5@nscdg.com>
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
 ************************************************************************************/

#include <px4_arch/spi_hw_description.h>
#include <drivers/drv_sensor.h>
// #include <nuttx/spi/spi.h>

#include <px4_platform_common/px4_config.h>

#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
//#include <unistd.h>

#include <nuttx/spi/spi.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "chip.h"
#include <mx8mn_gpio.h>
#include "board_config.h"
#include <systemlib/px4_macros.h>

#if defined(CONFIG_MX8MN_SPI1) || defined(CONFIG_MX8MN_SPI2) || defined(CONFIG_MX8MN_SPI3)

#define SPI_IBUS 2 // Internal Bus
#define SPI_EBUS 1 // External Bus

constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
    // initSPIBus(SPI::Bus::SPI0, {
    // 	initSPIDevice(SPIDEV_FLASH(0), SPI::CS{GPIO::PortC, GPIO::Pin2})
    // }),
    initSPIBus(SPI::Bus::SPI2,
               {
                   initSPIDevice(DRV_IMU_DEVTYPE_ICM42688P,
                                 SPI::CS{GPIO::Port3, GPIO::Pin24},
                                 SPI::DRDY{GPIO::Port3, GPIO::Pin23}),
                   initSPIDevice(DRV_GYR_DEVTYPE_BMI088,
                                 SPI::CS{GPIO::Port3, GPIO::Pin22},
                                 SPI::DRDY{GPIO::Port3, GPIO::Pin21}),
                   initSPIDevice(DRV_ACC_DEVTYPE_BMI088,
                                 SPI::CS{GPIO::Port5, GPIO::Pin13}),
                   // initSPIDevice(DRV_DEVTYPE_UNUSED, SPI::CS{GPIO::PortA,
                   // GPIO::Pin19}), // CAL Memory
               }
               // {GPIO::PortB, GPIO::Pin8} // Power enable
               ),
  initSPIBusExternal(SPI::Bus::SPI1, {
      initSPIConfigExternal(SPI::CS{GPIO::Port3, GPIO::Pin25})
      // initSPIConfigExternal(SPI::CS{GPIO::PortD, GPIO::Pin15}),
    }),
};

static constexpr bool unused = validateSPIConfig(px4_spi_buses);


/* * Macro to modify an existing GPIO config to be a Pull-Down Input.
 * 1. Mask out Mode (30-31), Output Value (29), and existing Pull bits (6 & 8)
 * 2. Set Mode to GPIO_INPUT (00)
 * 3. Set Pull Enable (PE) and leave PUE=0 for Pull-Down.
 */
#define PX4_SET_GPIO_PULLDOWN(conf) \
    (((conf) & ~(GPIO_MODE_MASK | GPIO_OUTPUT_ONE | PAD_CTL_PE | PAD_CTL_PUE)) | \
     GPIO_INPUT | PAD_CTL_PE)

/************************************************************************************
 * Public Functions
 ************************************************************************************/

__EXPORT void board_spi_reset(int ms, int bus_mask)
{
    /* 1. Set Chip Selects to inputs with pull-downs to avoid back-feeding */
    for (int bus = 0; bus < SPI_BUS_MAX_BUS_ITEMS; ++bus) {
        // Internal sensors on SPI2
      if (px4_spi_buses[bus].bus == PX4_BUS_NUMBER_TO_PX4(SPI_IBUS)) {
            for (int i = 0; i < SPI_BUS_MAX_DEVICES; ++i) {
                if (px4_spi_buses[bus].devices[i].cs_gpio != 0) {
                  mx8mn_gpio_config(PX4_SET_GPIO_PULLDOWN(
                      px4_spi_buses[bus].devices[i].cs_gpio));
                }
            }
        }
    }

    /* 2. Set DRDY inputs to pull-down */
    mx8mn_gpio_config(PX4_SET_GPIO_PULLDOWN(GPIO_BMI088_ACCEL_DRDY));
    mx8mn_gpio_config(PX4_SET_GPIO_PULLDOWN(GPIO_BMI088_GYRO_DRDY));
    mx8mn_gpio_config(PX4_SET_GPIO_PULLDOWN(GPIO_ICM42688_DRDY));

    /* 3. Power Down The Sensors */
    VDD_3V3_SENSORS_EN(false);
    up_mdelay(ms);

    /* 4. Power Up The Sensors */
    VDD_3V3_SENSORS_EN(true);
    // Give sensors time to stabilize (BMI088 needs at least 1ms to boot)
    up_mdelay(10); 

    /* 5. Restore all the CS to outputs (Inactive High) */
    for (int bus = 0; bus < SPI_BUS_MAX_BUS_ITEMS; ++bus) {
        if (px4_spi_buses[bus].bus == PX4_BUS_NUMBER_TO_PX4(SPI_IBUS)) {
            for (int i = 0; i < SPI_BUS_MAX_DEVICES; ++i) {
                if (px4_spi_buses[bus].devices[i].cs_gpio != 0) {
                    mx8mn_gpio_config(px4_spi_buses[bus].devices[i].cs_gpio);
                }
            }
        }
    }

    /* 6. Restore all the DRDY inputs to their original state (Pull-up) */
    mx8mn_gpio_config(GPIO_BMI088_ACCEL_DRDY);
    mx8mn_gpio_config(GPIO_BMI088_GYRO_DRDY);
    mx8mn_gpio_config(GPIO_ICM42688_DRDY);
}

/************************************************************************************
 * Name: mx8mn_spidev_initialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the NXP MX8MN-E board.
 *
 ************************************************************************************/

void mx8mn_spidev_initialize(void) {

    // /* 1. Mux the SPI2 Bus Pins (SCLK, MOSI, MISO) */
    //     mx8mn_iomuxc_config(IOMUXC_SPI2_CLK);
    //     mx8mn_iomuxc_config(IOMUXC_SPI2_MOSI);
    //     mx8mn_iomuxc_config(IOMUXC_SPI2_MISO);

	board_spi_reset(10, 0xffff);

	for (int bus = 0; bus < SPI_BUS_MAX_BUS_ITEMS; ++bus) {
		for (int i = 0; i < SPI_BUS_MAX_DEVICES; ++i) {
			if (px4_spi_buses[bus].devices[i].cs_gpio != 0) {
				mx8mn_gpio_config(px4_spi_buses[bus].devices[i].cs_gpio);
			}
		}
	}
}

/************************************************************************************
 * Name: mx8mn_spi_bus_initialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the NXP MX8MN v3 board.
 *
 ************************************************************************************/
static const px4_spi_bus_t *_spi_bus1;
static const px4_spi_bus_t *_spi_bus2;
static const px4_spi_bus_t *_spi_bus3;

__EXPORT int mx8mn_spi_bus_initialize(void)
{
	for (int i = 0; i < SPI_BUS_MAX_BUS_ITEMS; ++i) {
		switch (px4_spi_buses[i].bus) {
		// case PX4_BUS_NUMBER_TO_PX4(0): _spi_bus0 = &px4_spi_buses[i]; break;

		case PX4_BUS_NUMBER_TO_PX4(1): _spi_bus1 = &px4_spi_buses[i]; break;

		case PX4_BUS_NUMBER_TO_PX4(2): _spi_bus2 = &px4_spi_buses[i]; break;
		}
	}

	/* Configure SPI-based devices */

	struct spi_dev_s *spi_sensors = px4_spibus_initialize(PX4_BUS_NUMBER_TO_PX4(SPI_IBUS));

	if (!spi_sensors) {
		syslog(LOG_ERR, "[boot] FAILED to initialize SPI port %d\n", SPI_IBUS);
		return -ENODEV;
	}

	/* Default bus 1 to 1MHz and de-assert the known chip selects.
	 */

	SPI_SETFREQUENCY(spi_sensors, 1 * 1000 * 1000);
	SPI_SETBITS(spi_sensors, 8);
	SPI_SETMODE(spi_sensors, SPIDEV_MODE0);

	// /* Get the SPI port for the Memory */

	// struct spi_dev_s *spi_memory = px4_spibus_initialize(PX4_BUS_NUMBER_TO_PX4(0));

	// if (!spi_memory) {
	// 	syslog(LOG_ERR, "[boot] FAILED to initialize SPI port %d\n", 0);
	// 	return -ENODEV;
	// }

	/* Default bus 0 to 12MHz and de-assert the known chip selects.
	 */

	// SPI_SETFREQUENCY(spi_memory, 12 * 1000 * 1000);
	// SPI_SETBITS(spi_memory, 8);
	// SPI_SETMODE(spi_memory, SPIDEV_MODE3);

	/* Configure EXTERNAL SPI-based devices */

	struct spi_dev_s *spi_ext = px4_spibus_initialize(PX4_BUS_NUMBER_TO_PX4(SPI_EBUS));

	if (!spi_ext) {
		syslog(LOG_ERR, "[boot] FAILED to initialize SPI port %d\n", SPI_EBUS);
		return -ENODEV;
	}

	/* Default external bus to 1MHz and de-assert the known chip selects.
	 */

	SPI_SETFREQUENCY(spi_ext, 8 * 1000 * 1000);
	SPI_SETBITS(spi_ext, 8);
	SPI_SETMODE(spi_ext, SPIDEV_MODE3);

        /* 4. Deselect all devices on all initialized buses.
         * Logic Fix: We must call SELECT on the handle that owns the
         * device.
	 */
	for (int bus_idx = 0; bus_idx < SPI_BUS_MAX_BUS_ITEMS; ++bus_idx) {
	  struct spi_dev_s *handle = nullptr;
        
	  // Get the handle for the current bus in the loop
	  if (px4_spi_buses[bus_idx].bus == PX4_BUS_NUMBER_TO_PX4(2)) handle = spi_sensors;
	  if (px4_spi_buses[bus_idx].bus == PX4_BUS_NUMBER_TO_PX4(1)) handle = spi_ext;

	  if (handle) {
            for (int dev_idx = 0; dev_idx < SPI_BUS_MAX_DEVICES; ++dev_idx) {
	      if (px4_spi_buses[bus_idx].devices[dev_idx].cs_gpio != 0) {
		SPI_SELECT(handle, px4_spi_buses[bus_idx].devices[dev_idx].devid, false);
	      }
            }
	  }
	}

	return OK;

}

/**************************************************************************
 * Name:  mx8mn_spi[n]select, mx8mn_spi[n]status, and mx8mn_spi[n]cmddata
 *
 * Description:
 *   These external functions must be provided by board-specific logic.
 *   They are implementations of the select, status, and cmddata methods
 *   of the SPI interface defined by struct spi_ops_s
 *   (see include/nuttx/spi/spi.h).
 *   All other methods including mx8mn_spibus_initialize()) are provided
 *   by common mx8mn logic.
 *   To use this common SPI logic on your board:
 *
 *   1. Provide logic in mx8mn_board_initialize() to configure SPI chip
 *      select pins.
 *   2. Provide mx8mn_spi[n]select() and mx8mn_spi[n]status() functions
 *      in your board-specific logic. These functions will perform chip
 *      selection and status operations using GPIOs in the way your board
 *      is configured.
 *   3. If CONFIG_SPI_CMDDATA is defined in the NuttX configuration,
 *      provide mx8mn_spi[n]cmddata() functions in your board-specific
 *      logic. These functions will perform cmd/data selection operations
 *      using GPIOs in the way your board is configured.
 *   4. Add a call to mx8mn_spibus_initialize() in your low level
 *      application initialization logic
 *   5. The handle returned by mx8mn_spibus_initialize() may then be used
 *      to bind the SPI driver to higher level logic (e.g., calling
 *      mmcsd_spislotinitialize(), for example, will bind the SPI driver to
 *      the SPI MMC/SD driver).
 *
 *************************************************************************/

/**************************************************************************
 * Name: mx8mn_spixselect
 *
 * Description:
 * A generic helper to toggle the Chip Select pin for a specific device ID
 * on a given PX4 SPI bus configuration.
 **************************************************************************/

static inline void mx8mn_spixselect(const px4_spi_bus_t *bus,
				    struct spi_dev_s *dev,
                                    uint32_t devid, bool selected)
{
    if (!bus) return;

    for (int i = 0; i < SPI_BUS_MAX_DEVICES; ++i) {
        if (bus->devices[i].cs_gpio == 0) {
            break;
        }

        if (devid == bus->devices[i].devid) {
            /* * SPI select is active low.
             * selected == true (assert)  -> write 0
             * selected == false (de-assert) -> write 1
             */
            mx8mn_gpio_write(bus->devices[i].cs_gpio, !selected);
        }
    }
}

void mx8mn_spi1_select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
	spiinfo("devid: %d CS: %s\n", (int)devid, selected ? "assert" : "de-assert");
	mx8mn_spixselect(_spi_bus1, dev, devid, selected);
}

uint8_t mx8mn_spi1_status(FAR struct spi_dev_s *dev, uint32_t devid)
{
	return SPI_STATUS_PRESENT;
}

void mx8mn_spi2_select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
	spiinfo("devid: %d CS: %s\n", (int)devid, selected ? "assert" : "de-assert");
	mx8mn_spixselect(_spi_bus2, dev, devid, selected);
}

uint8_t mx8mn_spi2_status(FAR struct spi_dev_s *dev, uint32_t devid)
{
	return SPI_STATUS_PRESENT;
}

void mx8mn_spi3_select(FAR struct spi_dev_s *dev, uint32_t devid, bool selected)
{
	spiinfo("devid: %d CS: %s\n", (int)devid, selected ? "assert" : "de-assert");
	mx8mn_spixselect(_spi_bus3, dev, devid, selected);
}

uint8_t mx8mn_spi3_status(FAR struct spi_dev_s *dev, uint32_t devid)
{
	return SPI_STATUS_PRESENT;
}

#endif /* CONFIG_MX8MN_SPI1 || CONFIG_MX8MN_SPI2 || CONFIG_MX8MN_SPI3 */
