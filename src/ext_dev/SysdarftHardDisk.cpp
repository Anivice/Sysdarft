#include <SysdarftDebug.h>
#include <SysdarftHardDisk.h>

std::streamoff getFileSize(std::fstream& file)
{
    // Save current position (if needed)
    const std::streampos currentPos = file.tellg();

    // Seek to the end to determine file size
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();

    // Restore the pointer to the original position (optional)
    file.seekg(currentPos);

    return size;
}

class SysdarftDiskError final : public SysdarftBaseError {
public:
    explicit SysdarftDiskError(const std::string & msg) : SysdarftBaseError(msg) { }
};

SysdarftHardDisk::SysdarftHardDisk(const std::string &file_name)
{
    _sysdarftHardDiskFile.open(file_name);
    if (!_sysdarftHardDiskFile.is_open()) {
        throw SysdarftDiskError("Cannot open file " + file_name);
    }

    device_buffer.emplace(HDD_CMD_REQUEST_RD, ControllerDataStream());
    device_buffer.emplace(HDD_CMD_REQUEST_WR, ControllerDataStream());
    device_buffer.emplace(HDD_DEVICE_SIZE,    ControllerDataStream());
    device_buffer.emplace(HDD_REQUEST_PARAM,  ControllerDataStream());
}

bool SysdarftHardDisk::request_read(const uint64_t port)
{
    const uint64_t size = getFileSize(_sysdarftHardDiskFile);

    if (port == HDD_DEVICE_SIZE)
    {
        device_buffer.at(port).push(size);
    }
    else if (port == HDD_CMD_REQUEST_RD)
    {
        if (const uint64_t ops_end_sector = (start_sector + sector_count) * 512;
            ops_end_sector > size)
        {
            return false;
        }

        const uint64_t start_off = start_sector * 512;
        const uint64_t length = sector_count * 512;

        if (length == 0) {
            return false;
        }

        // Seek to the specified offset
        _sysdarftHardDiskFile.seekg(static_cast<long>(start_off), std::ios::beg);
        if (!_sysdarftHardDiskFile) {
            return false;
        }

        auto & buffer = device_buffer.at(port).device_buffer;
        buffer.resize(length);

        // Read 'length' bytes from the file
        _sysdarftHardDiskFile.read((char*)buffer.data(), static_cast<long>(length));

        if (const std::streamsize bytesRead = _sysdarftHardDiskFile.gcount();
            bytesRead < static_cast<std::streamsize>(length))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool SysdarftHardDisk::request_write(const uint64_t port)
{
    const uint64_t size = getFileSize(_sysdarftHardDiskFile);
    if (port == HDD_REQUEST_PARAM)
    {
        start_sector = device_buffer.at(port).pop<uint64_t>();
        sector_count = device_buffer.at(port).pop<uint64_t>();
        return true;
    }
    else if (port == HDD_CMD_REQUEST_WR)
    {
        if (const uint64_t ops_end_sector = (start_sector + sector_count) * 512;
            ops_end_sector > size)
        {
            return false;
        }

        const uint64_t start_off = start_sector * 512;
        const uint64_t length = sector_count * 512;
        if (length == 0) {
            return false;
        }

        auto & buffer = device_buffer.at(port).device_buffer;

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

        buffer.clear();
        return true;
    }

    return false;
}
