#include <px4_platform_common/px4_config.h>          // __EXPORT, __BEGIN_DECLS
#include <px4_platform_common/board_common.h>        // board_bus_types, UUID/GUID APIs & lengths

// #include <px4_platform_common/i2c.h>                 // px4_i2c_bus_t, PX4_NUMBER_I2C_BUSES

// #include <nuttx/i2c/i2c_master.h>                    // struct i2c_master_s
// #include <string.h>                                  // memset
// #include <stdint.h>

// __EXPORT extern const px4_i2c_bus_t px4_i2c_buses[PX4_NUMBER_I2C_BUSES] = { px4_i2c_bus_t{} };

__BEGIN_DECLS

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

