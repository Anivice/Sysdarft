/* SysdarftDisks.h
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

#ifndef SYSDARFTHARDDISK_H
#define SYSDARFTHARDDISK_H

#include <SysdarftIOHub.h>

#define FDA_REG_SIZE        (0x116)
#define FDA_REG_START_SEC   (0x117)
#define FDA_REG_SEC_COUNT   (0x118)
#define FDA_CMD_REQUEST_RD  (0x119)
#define FDA_CMD_REQUEST_WR  (0x11A)

#define FDB_REG_SIZE        (0x126)
#define FDB_REG_START_SEC   (0x127)
#define FDB_REG_SEC_COUNT   (0x128)
#define FDB_CMD_REQUEST_RD  (0x129)
#define FDB_CMD_REQUEST_WR  (0x12A)

#define HDD_REG_SIZE        (0x136)
#define HDD_REG_START_SEC   (0x137)
#define HDD_REG_SEC_COUNT   (0x138)
#define HDD_CMD_REQUEST_RD  (0x139)
#define HDD_CMD_REQUEST_WR  (0x13A)

class SysdarftDiskError final : public SysdarftDeviceIOError {
public:
    explicit SysdarftDiskError(const std::string & msg) : SysdarftDeviceIOError(msg) { }
};

int lock_file(int fd, int cmd, int type);

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
class SYSDARFT_EXPORT_SYMBOL SysdarftDiskImager : public SysdarftExternalDeviceBaseClass
{
private:
    int _sysdarftHardDiskFile;
    uint64_t start_sector = 0;
    uint64_t sector_count = 0;
    const uint64_t device_size = 0;

public:
    explicit SysdarftDiskImager(const std::string & file_name);
    ~SysdarftDiskImager() noexcept override;
    bool request_read(uint64_t) override;
    bool request_write(uint64_t) override;
};

class SYSDARFT_EXPORT_SYMBOL SysdarftBlockDevices final : public SysdarftDiskImager
    <   HDD_REG_SIZE,
        HDD_REG_START_SEC,
        HDD_REG_SEC_COUNT,
        HDD_CMD_REQUEST_RD,
        HDD_CMD_REQUEST_WR >
{
public:
    explicit SysdarftBlockDevices(const std::string & file_name) : SysdarftDiskImager(file_name) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftFloppyDiskA final : public SysdarftDiskImager
    <   FDA_REG_SIZE,
        FDA_REG_START_SEC,
        FDA_REG_SEC_COUNT,
        FDA_CMD_REQUEST_RD,
        FDA_CMD_REQUEST_WR >
{
public:
    explicit SysdarftFloppyDiskA(const std::string & file_name) : SysdarftDiskImager(file_name) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftFloppyDiskB final : public SysdarftDiskImager
    <   FDB_REG_SIZE,
        FDB_REG_START_SEC,
        FDB_REG_SEC_COUNT,
        FDB_CMD_REQUEST_RD,
        FDB_CMD_REQUEST_WR >
{
public:
    explicit SysdarftFloppyDiskB(const std::string & file_name) : SysdarftDiskImager(file_name) { }
};

ssize_t SYSDARFT_EXPORT_SYMBOL getFileSize(int);

#include "SysdarftDisks.inl"

#endif //SYSDARFTHARDDISK_H
