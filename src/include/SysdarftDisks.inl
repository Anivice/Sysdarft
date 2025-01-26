/* SysdarftDisks.inl
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

#ifndef SYSDARFTDISKS_INL
#define SYSDARFTDISKS_INL

#include "SysdarftDisks.h"
#include <ext/stdio_filebuf.h> // For __gnu_cxx::stdio_filebuf
#include <fcntl.h>
#include <cstring>

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
SysdarftDiskImager(const std::string &file_name)
{
    _sysdarftHardDiskFile = open(file_name.c_str(), O_RDWR | O_SYNC | O_CLOEXEC);
    if (_sysdarftHardDiskFile == -1) {
        throw SysdarftDiskError("Cannot open file " + file_name);
    }

    if (lock_file(_sysdarftHardDiskFile, F_SETLK, F_WRLCK) == -1) {
        throw SysdarftDiskError("Failed to lock file " + file_name + ", possibly used by another process?");
    }

    device_buffer.emplace(REG_SIZE,         std::make_unique<ControllerDataStream>());
    device_buffer.emplace(REG_START_SEC,    std::make_unique<ControllerDataStream>());
    device_buffer.emplace(REG_SEC_COUNT,    std::make_unique<ControllerDataStream>());
    device_buffer.emplace(CMD_REQUEST_RD,   std::make_unique<ControllerDataStream>());
    device_buffer.emplace(CMD_REQUEST_WR,   std::make_unique<ControllerDataStream>());

    (*(uint64_t*)&device_size) = getFileSize(_sysdarftHardDiskFile);
}

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
~SysdarftDiskImager() noexcept
{
    // try unlock file
    if (lock_file(_sysdarftHardDiskFile, F_SETLK, F_UNLCK) == -1)
    {
        if (debug::verbose) {
            std::cerr << "Unlock file failed for descriptor " << std::to_string(_sysdarftHardDiskFile) << std::endl;
            std::cerr << "Errno: " << errno << ": " << std::strerror(errno) << std::endl;
        }
    }

    // close
    close(_sysdarftHardDiskFile);
}

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
bool
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
request_read(const uint64_t port)
{
    if (port == REG_SIZE)
    {
        device_buffer.at(port)->push(device_size / 512);
        return true;
    }
    else if (port == CMD_REQUEST_RD)
    {
        if (const uint64_t ops_end_sector = (start_sector + sector_count) * 512;
            ops_end_sector > device_size)
        {
            return false;
        }

        const uint64_t start_off = start_sector * 512;
        const uint64_t length = sector_count * 512;

        if (length == 0 || start_off + length > device_size) {
            return false;
        }

        // Seek to the specified offset
        if (lseek64(_sysdarftHardDiskFile, start_off, SEEK_SET) == -1) {
            return false;
        }

        std::vector<uint8_t> buffer;
        buffer.resize(length);

        // Read 'length' bytes from the file
        const auto read_len = read(_sysdarftHardDiskFile, buffer.data(), static_cast<long>(length));

        if (read_len != static_cast<std::streamsize>(length)) {
            return false;
        }

        device_buffer.at(port)->insert(buffer);

        return true;
    }

    return false;
}

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
bool
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
request_write(const uint64_t port)
{
    if (port == REG_START_SEC)
    {
        start_sector = device_buffer.at(port)->pop<uint64_t>();
        return true;
    }
    else if (port == REG_SEC_COUNT)
    {
        sector_count = device_buffer.at(port)->pop<uint64_t>();
        return true;
    }
    else if (port == CMD_REQUEST_WR)
    {
        if (const uint64_t ops_end_sector = (start_sector + sector_count) * 512;
            ops_end_sector > device_size)
        {
            return false;
        }

        const uint64_t start_off = start_sector * 512;
        const uint64_t length = sector_count * 512;
        if (length == 0 || start_off + length > device_size) {
            return false;
        }

        auto buffer = device_buffer.at(port)->getObject();

        if (length != buffer.size()) {
            return false;
        }

        // Seek to the specified offset
        if (lseek64(_sysdarftHardDiskFile, start_off, SEEK_SET) == -1) {
            return false;
        }

        const auto write_len = write(_sysdarftHardDiskFile, buffer.data(), static_cast<long>(length));
        if (write_len != static_cast<std::streamsize>(length)) {
            return false;
        }

        // flash device
        fsync(_sysdarftHardDiskFile);

        device_buffer.at(port)->clear();
        return true;
    }

    return false;
}

#endif //SYSDARFTDISKS_INL
