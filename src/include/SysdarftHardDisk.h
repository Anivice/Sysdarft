#ifndef SYSDARFTHARDDISK_H
#define SYSDARFTHARDDISK_H

#include <fstream>
#include <SysdarftIOHub.h>

#define HDD_REG_SIZE        (0x136)
#define HDD_REG_START_SEC   (0x137)
#define HDD_REG_SEC_COUNT   (0x138)
#define HDD_CMD_REQUEST_RD  (0x139)
#define HDD_CMD_REQUEST_WR  (0x13A)

class SYSDARFT_EXPORT_SYMBOL SysdarftHardDisk final : public SysdarftExternalDeviceBaseClass
{
private:
    std::fstream _sysdarftHardDiskFile;
    uint64_t start_sector = 0;
    uint64_t sector_count = 0;

public:
    explicit SysdarftHardDisk(const std::string & file_name);
    bool request_read(uint64_t) override;
    bool request_write(uint64_t) override;
};

#endif //SYSDARFTHARDDISK_H
