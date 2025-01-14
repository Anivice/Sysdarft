#ifndef SYSDARFTHARDDISK_H
#define SYSDARFTHARDDISK_H

#include <fstream>
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

template <  unsigned REG_SIZE,
            unsigned REG_START_SEC,
            unsigned REG_SEC_COUNT,
            unsigned CMD_REQUEST_RD,
            unsigned CMD_REQUEST_WR >
class SYSDARFT_EXPORT_SYMBOL SysdarftDiskImager : public SysdarftExternalDeviceBaseClass
{
private:
    std::fstream _sysdarftHardDiskFile;
    uint64_t start_sector = 0;
    uint64_t sector_count = 0;

public:
    explicit SysdarftDiskImager(const std::string & file_name);
    bool request_read(uint64_t) override;
    bool request_write(uint64_t) override;
};

class SYSDARFT_EXPORT_SYMBOL SysdarftHardDisk final : public SysdarftDiskImager
    <   HDD_REG_SIZE,
        HDD_REG_START_SEC,
        HDD_REG_SEC_COUNT,
        HDD_CMD_REQUEST_RD,
        HDD_CMD_REQUEST_WR >
{
public:
    explicit SysdarftHardDisk(const std::string & file_name) : SysdarftDiskImager(file_name) { }
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

std::streamoff SYSDARFT_EXPORT_SYMBOL getFileSize(std::fstream&);

#include "SysdarftDisks.inl"

#endif //SYSDARFTHARDDISK_H
