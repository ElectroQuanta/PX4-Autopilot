#include <px4_platform_common/px4_config.h>          // __EXPORT, __BEGIN_DECLS
#include <px4_platform_common/board_common.h>        // board_bus_types, UUID/GUID APIs & lengths

#include <px4_platform_common/i2c.h>                 // px4_i2c_bus_t, PX4_NUMBER_I2C_BUSES
#include <px4_platform_common/spi.h>                 // px4_spi_bus_t, PX4_NUMBER_SPI_BUSES

#include <nuttx/i2c/i2c_master.h>                    // struct i2c_master_s
#include <string.h>                                  // memset
#include <stdint.h>

__EXPORT extern const px4_i2c_bus_t px4_i2c_buses[PX4_NUMBER_I2C_BUSES] = { px4_i2c_bus_t{} };
// __EXPORT extern const px4_spi_bus_t px4_spi_buses[PX4_NUMBER_SPI_BUSES] = { px4_spi_bus_t{} };

__BEGIN_DECLS

/* Bus tables
 * - Keep at least one sentinel entry (bus defaults to -1 in the C++ struct)
 * - Remaining entries are value-initialized as well.
 */
/* Force external linkage so __EXPORT visibility is meaningful (no -Wattributes). */
/* Minimal bus presence hook: no buses enabled yet */
// __EXPORT bool board_has_bus(enum board_bus_types type, uint32_t bus)
// {
//     (void)type;
//     (void)bus;
//     return false;
// }

/* Minimal I2C init hooks: no I2C yet */
__EXPORT FAR struct i2c_master_s *px4_i2cbus_initialize(int bus)
{
	(void)bus;
	return NULL;
}

__EXPORT int px4_i2cbus_uninitialize(FAR struct i2c_master_s *dev)
{
	(void)dev;
	return 0;
}

/* UUID/GUID stubs */
__EXPORT void px4_cpu_uuid_get(uint32_t *uuid_words)
{
	if (!uuid_words) {
		return;
	}

	for (unsigned i = 0; i < PX4_CPU_UUID_WORD32_LENGTH; i++) {
		uuid_words[i] = 0;
	}
}

__EXPORT int px4_guid_get(uint8_t *guid)
{
	if (!guid) {
		return -1;
	}

	memset(guid, 0, PX4_GUID_BYTE_LENGTH);
	return PX4_GUID_BYTE_LENGTH;
}

__EXPORT void px4_cpu_mfguid_get(uint32_t *mfguid_words)
{
	if (!mfguid_words) {
		return;
	}

	for (unsigned i = 0; i < PX4_CPU_MFGUID_WORD32_LENGTH; i++) {
		mfguid_words[i] = 0;
	}
}

__END_DECLS

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>

#include <stdlib.h>
#include <malloc.h>

__BEGIN_DECLS

__EXPORT void *px4_cache_aligned_alloc(size_t size)
{
	const size_t a = 32;                    // OK for early bring-up
	const size_t n = (size + (a - 1u)) & ~(a - 1u);
	return memalign(a, n);
}

__EXPORT void px4_cache_aligned_free(void *p)
{
	free(p);
}

__END_DECLS
