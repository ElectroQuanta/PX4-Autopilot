/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
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
#pragma once

#include <nuttx/compiler.h>
#include <nuttx/irq.h>
#include <stdint.h>

__BEGIN_DECLS


/* For minimal bring-up:
 * - we tell PX4 there is 1 dummy I2C bus entry
 * - 0 SPI buses
 * You can refine these later once you actually hook up I2C/SPI.
 */

#define PX4_NUMBER_I2C_BUSES   1
#define PX4_NUMBER_SPI_BUSES 1


/* PX4 expects these lengths to exist at compile time */
#define PX4_CPU_UUID_BYTE_LENGTH       16
#define PX4_CPU_UUID_WORD32_LENGTH     (PX4_CPU_UUID_BYTE_LENGTH / 4)

#define PX4_CPU_MFGUID_BYTE_LENGTH     16
#define PX4_CPU_MFGUID_WORD32_LENGTH   (PX4_CPU_MFGUID_BYTE_LENGTH / 4)

/* PX4 critical section wrappers used by parameters, etc. */
static inline irqstate_t px4_enter_critical_section(void)
{
  return enter_critical_section();
}

static inline void px4_leave_critical_section(irqstate_t flags)
{
  leave_critical_section(flags);
}

/* Board HW type string used by src/lib/version */
__EXPORT const char *board_get_hw_type_name(void);

/* CPU IDs (you can keep them zero for now) */
__EXPORT void px4_cpu_uuid_get(uint32_t *uuid_words);
__EXPORT void px4_cpu_mfguid_get(uint32_t *mfguid_words);

__END_DECLS

