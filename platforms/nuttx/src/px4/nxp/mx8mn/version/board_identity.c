/****************************************************************************
 *
 *   Copyright (C) 2017 PX4 Development Team. All rights reserved.
 *   Author: @author David Sidrane <david_s5@nscdg.com>
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
 * @file board_identity.c
 * Implementation of Kientis based Board identity API
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/board_common.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef PX4_SOC_ARCH_ID
#define PX4_SOC_ARCH_ID 0x0000
#endif

/* For early bring-up:
 * - UUID/MFGUID are all zeros
 * - PX4 GUID starts with SOC_ARCH_ID, then zeros
 *
 * Later you can replace the zero fill with a real unique ID source
 * (e.g. OCOTP/eFuses if accessible to the M7 via your RDC setup).
 */

static const uint16_t soc_arch_id = PX4_SOC_ARCH_ID;

void board_get_uuid(uuid_byte_t uuid_bytes)
{
	memset(uuid_bytes, 0, PX4_CPU_UUID_BYTE_LENGTH);
}

void board_get_uuid32(uuid_uint32_t uuid_words)
{
	memset(uuid_words, 0, PX4_CPU_UUID_WORD32_LENGTH * sizeof(uint32_t));
}

int board_get_uuid32_formated(char *format_buffer, int size,
			      const char *format,
			      const char *separator)
{
	uuid_uint32_t uuid;
	board_get_uuid32(uuid);

	int offset = 0;
	const int sep_size = separator ? (int)strlen(separator) : 0;

	for (unsigned i = 0; (offset < size - 1) && (i < PX4_CPU_UUID_WORD32_LENGTH); i++) {
		offset += snprintf(&format_buffer[offset], size - offset, format, uuid[i]);

		if (sep_size && (offset < size - sep_size - 1) && (i < PX4_CPU_UUID_WORD32_LENGTH - 1)) {
			strncat(&format_buffer[offset], separator, size - offset);
			offset += sep_size;
		}
	}

	return 0;
}

int board_get_mfguid(mfguid_t mfgid)
{
	memset(mfgid, 0, PX4_CPU_MFGUID_BYTE_LENGTH);
	return PX4_CPU_MFGUID_BYTE_LENGTH;
}

int board_get_mfguid_formated(char *format_buffer, int size)
{
	mfguid_t mfguid;
	board_get_mfguid(mfguid);

	int offset = 0;

	for (unsigned i = 0; i < PX4_CPU_MFGUID_BYTE_LENGTH && offset < size - 1; i++) {
		offset += snprintf(&format_buffer[offset], size - offset, "%02x", mfguid[i]);
	}

	return offset;
}

int board_get_px4_guid(px4_guid_t px4_guid)
{
	/* PX4 GUID = 2 bytes SOC_ARCH_ID + UUID bytes */
	uint8_t *pb = (uint8_t *)&px4_guid[0];

	*pb++ = (soc_arch_id >> 8) & 0xff;
	*pb++ = (soc_arch_id & 0xff);

	memset(pb, 0, PX4_CPU_UUID_BYTE_LENGTH);
	return PX4_GUID_BYTE_LENGTH;
}

int board_get_px4_guid_formated(char *format_buffer, int size)
{
	px4_guid_t px4_guid;
	board_get_px4_guid(px4_guid);

	int offset = 0;

	/* Require odd size (2 chars per byte + '\0'), discard from MSD */
	size = (size & 1) ? size : size - 1;

	for (unsigned i = PX4_GUID_BYTE_LENGTH - (unsigned)(size / 2);
	     offset < size && i < PX4_GUID_BYTE_LENGTH; i++) {
		offset += snprintf(&format_buffer[offset], size - offset, "%02x", px4_guid[i]);
	}

	return offset;
}
