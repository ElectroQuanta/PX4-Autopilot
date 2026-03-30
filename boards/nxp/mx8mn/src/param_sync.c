/****************************************************************************
 * boards/nxp/mx8mn/src/param_sync.c
 *
 *   Copyright (c) 2026 PX4 Development Team. All rights reserved.
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

#include <nuttx/config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>

#include <px4_platform_common/module.h>

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
  int ret = 0;
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

static void mirror_logs(void)
{
  DIR *log_dir = opendir("/fs/microsd/log");
  if (log_dir == NULL) return;

  struct dirent *sess_entry;
  while ((sess_entry = readdir(log_dir)) != NULL)
    {
      if (sess_entry->d_type != DTYPE_DIRECTORY) continue;
      if (strcmp(sess_entry->d_name, ".") == 0 || strcmp(sess_entry->d_name, "..") == 0) continue;

      char ram_sess_path[128];
      char lin_sess_path[128];
      snprintf(ram_sess_path, sizeof(ram_sess_path), "/fs/microsd/log/%s", sess_entry->d_name);
      snprintf(lin_sess_path, sizeof(lin_sess_path), "/mnt/linux/log/%s", sess_entry->d_name);

      mkdir("/mnt/linux/log", 0777);
      mkdir(lin_sess_path, 0777);

      DIR *sess_dir = opendir(ram_sess_path);
      if (sess_dir == NULL) continue;

      struct dirent *log_entry;
      while ((log_entry = readdir(sess_dir)) != NULL)
        {
          if (log_entry->d_type != DTYPE_FILE) continue;
          if (strstr(log_entry->d_name, ".ulg") == NULL) continue;

          char ram_file[256];
          char lin_file[256];
          snprintf(ram_file, sizeof(ram_file), "%s/%s", ram_sess_path, log_entry->d_name);
          snprintf(lin_file, sizeof(lin_file), "%s/%s", lin_sess_path, log_entry->d_name);

          struct stat st_ram, st_lin;
          if (stat(ram_file, &st_ram) == 0)
            {
              if (stat(lin_file, &st_lin) != 0 || st_ram.st_size > st_lin.st_size)
                {
                  copy_file(ram_file, lin_file);
                }
            }
        }
      closedir(sess_dir);
    }
  closedir(log_dir);
}

int param_sync_thread(int argc, char *argv[])
{
  struct stat prev_param = {0};
  struct stat prev_cal   = {0};
  struct stat curr;

  sleep(15);

  while (1)
    {
      sleep(5);

      if (stat("/fs/mtd_params", &curr) == 0)
        {
          if (curr.st_mtime != prev_param.st_mtime || curr.st_size != prev_param.st_size)
            {
              if (copy_file("/fs/mtd_params", "/mnt/linux/mtd_params") == 0)
                {
                  prev_param = curr;
                }
            }
        }

      if (stat("/fs/mtd_caldata", &curr) == 0)
        {
          if (curr.st_mtime != prev_cal.st_mtime || curr.st_size != prev_cal.st_size)
            {
              if (copy_file("/fs/mtd_caldata", "/mnt/linux/mtd_caldata") == 0)
                {
                  prev_cal = curr;
                }
            }
        }

      mirror_logs();
    }

  return 0;
}
#endif /* CONFIG_MX8MN_RPMSG */
