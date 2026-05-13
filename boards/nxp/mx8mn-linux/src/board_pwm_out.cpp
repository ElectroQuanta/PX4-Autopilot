/****************************************************************************
 *
 *   Copyright (c) 2024 PX4 Development Team. All rights reserved.
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
#ifndef MODULE_NAME
#define MODULE_NAME "mx8mn_sysfs_pwm_out"
#endif

#include "board_pwm_out.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <px4_platform_common/log.h>

using namespace pwm_out;

const char *MX8MNSysfsPWMOut::_device_chips[MAX_NUM_PWM] = {
	"/sys/class/pwm/pwmchip0",
	"/sys/class/pwm/pwmchip1",
	"/sys/class/pwm/pwmchip2",
	"/sys/class/pwm/pwmchip3"
};

MX8MNSysfsPWMOut::MX8MNSysfsPWMOut(int max_num_outputs)
{
	if (max_num_outputs > MAX_NUM_PWM) {
		if (max_num_outputs != 16) {
			PX4_WARN("number of outputs too large. Setting to %i", MAX_NUM_PWM);
		}

		max_num_outputs = MAX_NUM_PWM;
	}

	for (int i = 0; i < MAX_NUM_PWM; ++i) {
		_pwm_fd[i] = -1;
	}

	_pwm_num = max_num_outputs;
}

MX8MNSysfsPWMOut::~MX8MNSysfsPWMOut()
{
	for (int i = 0; i < MAX_NUM_PWM; ++i) {
		if (_pwm_fd[i] != -1) {
			::close(_pwm_fd[i]);
		}
	}
}

int MX8MNSysfsPWMOut::init()
{
	int i;
	char path[128];

	for (i = 0; i < _pwm_num; ++i) {
		::snprintf(path, sizeof(path), "%s/export", _device_chips[i]);
		pwm_write_sysfs(path, 0);
	}

	for (i = 0; i < _pwm_num; ++i) {
		::snprintf(path, sizeof(path), "%s/pwm0/period", _device_chips[i]);

		if (pwm_write_sysfs(path, (int)1e9 / FREQUENCY_PWM) < 0) {
			PX4_ERR("PWM%d period failed", i);
		}
	}

	for (i = 0; i < _pwm_num; ++i) {
		::snprintf(path, sizeof(path), "%s/pwm0/enable", _device_chips[i]);

		if (pwm_write_sysfs(path, 1) < 0) {
			PX4_ERR("PWM%d enable failed", i);
		}
	}

	for (i = 0; i < _pwm_num; ++i) {
		::snprintf(path, sizeof(path), "%s/pwm0/duty_cycle", _device_chips[i]);
		_pwm_fd[i] = ::open(path, O_WRONLY | O_CLOEXEC);

		if (_pwm_fd[i] == -1) {
			PX4_ERR("PWM%d: Failed to open duty_cycle.", i);
			return -errno;
		}
	}

	return 0;
}

int MX8MNSysfsPWMOut::send_output_pwm(const uint16_t *pwm, int num_outputs)
{
	char data[16];

	if (num_outputs > _pwm_num) {
		num_outputs = _pwm_num;
	}

	int ret = 0;

	for (int i = 0; i < num_outputs; ++i) {
		int n = ::snprintf(data, sizeof(data), "%u", pwm[i] * 1000);
		int write_ret = ::write(_pwm_fd[i], data, n);

		if (n != write_ret) {
			ret = -1;
		}
	}

	return ret;
}

int MX8MNSysfsPWMOut::pwm_write_sysfs(const char *path, int value)
{
	int fd = ::open(path, O_WRONLY | O_CLOEXEC);
	int n;
	char data[16];

	if (fd == -1) {
		return -errno;
	}

	n = ::snprintf(data, sizeof(data), "%u", value);

	if (n > 0) {
		n = ::write(fd, data, n);
	}

	::close(fd);

	return 0;
}
