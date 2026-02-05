/****************************************************************************
 *
 *   Copyright (c) 2016-2017 PX4 Development Team. All rights reserved.
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
 * @file hrt.c
 * Author: David Sidrane <david_s5@nscdg.com>
 *
 * High-resolution timer callouts and timekeeping.
 *
 * This can use any Kinetis TPM timer.
 *
 * Note that really, this could use systick too, but that's
 * monopolised by NuttX and stealing it would just be awkward.
 *
 * We don't use the NuttX Kinetis driver per se; rather, we
 * claim the timer and then drive it directly.
 */

#include <drivers/drv_hrt.h>
#include <nuttx/clock.h>
#include <time.h>
#include <string.h>
#include <nuttx/irq.h>
#include <px4_platform_common/px4_config.h>
#include <sys/types.h>

/**
 * @brief Stubs for Logger
 *
 * Taken from Kinetis hrt.c
 */
/* latency histogram */
const uint16_t latency_bucket_count = LATENCY_BUCKET_COUNT;
const uint16_t latency_buckets[LATENCY_BUCKET_COUNT] = { 1, 2, 5, 10, 20, 50, 100, 1000 };
__EXPORT uint32_t latency_counters[LATENCY_BUCKET_COUNT + 1];

/* Minimal HRT for early boot:
 * - provides monotonic time in microseconds
 * - provides callout APIs as no-ops (enough to link and print banner)
 *
 * WARNING: Many PX4 modules rely on scheduled callouts; keep your .px4board minimal.
 */

hrt_abstime hrt_absolute_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    const uint64_t usec = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
    return (hrt_abstime)usec;
}

void hrt_init(void) { }

void hrt_call_init(struct hrt_call *entry)
{
    if (entry) { memset(entry, 0, sizeof(*entry)); }
}

void hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg)
{
    (void)entry; (void)delay; (void)callout; (void)arg;
}

void hrt_call_at(struct hrt_call *entry, hrt_abstime calltime, hrt_callout callout, void *arg)
{
    (void)entry; (void)calltime; (void)callout; (void)arg;
}

void hrt_call_every(struct hrt_call *entry, hrt_abstime delay, hrt_abstime interval, hrt_callout callout, void *arg)
{
    (void)entry; (void)delay; (void)interval; (void)callout; (void)arg;
}

void hrt_cancel(struct hrt_call *entry)
{
    if (entry) { entry->deadline = 0; entry->period = 0; }
}

bool hrt_called(struct hrt_call *entry)
{
    return entry ? (entry->deadline == 0) : true;
}


/**
 * Store the absolute time in an interrupt-safe fashion
 */
void
hrt_store_absolute_time(volatile hrt_abstime *t)
{
	irqstate_t flags = px4_enter_critical_section();
	*t = hrt_absolute_time();
	px4_leave_critical_section(flags);
}
