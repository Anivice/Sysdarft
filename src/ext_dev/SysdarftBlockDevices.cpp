/* SysdarftHardDisk.cpp
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <SysdarftDebug.h>
#include <SysdarftDisks.h>

ssize_t SYSDARFT_EXPORT_SYMBOL getFileSize(int fd)
{
    struct stat file_stat{};
    if (fstat(fd, &file_stat) == -1) {
        throw std::runtime_error("Failed to get file size");
    }

    // Get the file size from the st_size field
    return file_stat.st_size;
}

int lock_file(const int fd, const int cmd, const int type)
{
    flock fl{};

    fl.l_type = static_cast<short>(type);       // Type of lock: F_RDLCK, F_WRLCK, F_UNLCK
    fl.l_whence = SEEK_SET; // Start of the file
    fl.l_start = 0;         // Offset from l_whence
    fl.l_len = 0;           // Lock the entire file
    fl.l_pid = getpid();    // PID of the process

    if (fcntl(fd, cmd, &fl) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}
