#ifndef SYSDARFTDISKS_INL
#define SYSDARFTDISKS_INL

#include "SysdarftDisks.h"

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
SysdarftDiskImager(const std::string &file_name)
{
    _sysdarftHardDiskFile.open(file_name);
    if (!_sysdarftHardDiskFile.is_open()) {
        throw SysdarftDiskError("Cannot open file " + file_name);
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
bool
SysdarftDiskImager < REG_SIZE, REG_START_SEC, REG_SEC_COUNT, CMD_REQUEST_RD, CMD_REQUEST_WR > ::
request_read(const uint64_t port)
{
    if (port == REG_SIZE)
    {
        device_buffer.at(port)->push(device_size);
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
        _sysdarftHardDiskFile.seekg(static_cast<long>(start_off), std::ios::beg);
        if (!_sysdarftHardDiskFile) {
            return false;
        }

        std::vector<uint8_t> buffer;
        buffer.resize(length);

        // Read 'length' bytes from the file
        _sysdarftHardDiskFile.read((char*)buffer.data(), static_cast<long>(length));

        if (const std::streamsize bytesRead = _sysdarftHardDiskFile.gcount();
            bytesRead < static_cast<std::streamsize>(length))
        {
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
        _sysdarftHardDiskFile.seekp(static_cast<long>(start_off), std::ios::beg);
        if (!_sysdarftHardDiskFile) {
            return false;
        }

        _sysdarftHardDiskFile.write((char*)buffer.data(), static_cast<long>(length));
        if (!_sysdarftHardDiskFile) {
            return false;
        }

        device_buffer.at(port)->clear();
        return true;
    }

    return false;
}

#endif //SYSDARFTDISKS_INL
