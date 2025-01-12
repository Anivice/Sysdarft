#ifndef SYSDARFTMEMORY_H
#define SYSDARFTMEMORY_H

#include <SysdarftDebug.h>

/*
 * Memory Layout:
 * Lower 1 MB Firmware Data
 * 0x00000 - 0x9FFFF [BOOT CODE]     - 640KB
 * 0xA0000 - 0xC17FF [CONFIGURATION] - 134KB
 *                    - 0xA0000 - 0xA0FFF [4KB Interruption Table: 256 Interrupts]
 * 0xC1800 - 0xFFFFF [FIRMWARE]      - 250KB
 */
#define BOOT_LOADER_START   (0x00000)
#define BOOT_LOADER_END     (0x9FFFF)
#define BOOT_LOADER_SIZE    (BOOT_LOADER_END - BOOT_LOADER_START + 1)
#define INTERRUPTION_VECTOR (0xA0000)
#define INTERRUPTION_VEC_ED (0xA0FFF)
#define INTERRUPTION_VEC_LN (INTERRUPTION_VEC_ED - INTERRUPTION_VECTOR + 1)
#define BIOS_START          (0xC1800)
#define BIOS_END            (0xFFFFF)
#define BIOS_SIZE           (BIOS_END - BIOS_START + 1)

#define BLOCK_SIZE 4096

class IllegalMemoryAccessException final : public SysdarftBaseError
{
public:
    explicit IllegalMemoryAccessException(const std::string & msg) :
        SysdarftBaseError("Illegal memory access: " + msg) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUMemoryAccess
{
protected:
    std::mutex MemoryAccessMutex;
    std::vector < std::array < uint8_t, BLOCK_SIZE > > Memory;
    std::atomic<uint64_t> TotalMemory = 32 * 1024 * 1024; // 32MB Memory

    SysdarftCPUMemoryAccess();
    void read_memory(uint64_t address, char * _dest, uint64_t size);
    void write_memory(uint64_t address, const char* _source, uint64_t size);

    template < typename DataType >
    void push_memory_to(const uint64_t begin, uint64_t & offset, const DataType & val)
    {
        offset -= sizeof(DataType);
        write_memory(begin + offset, (char*)&val, sizeof(DataType));
    }

    void mpush8(const uint64_t begin, uint64_t & offset, uint8_t value) {
        push_memory_to<uint8_t>(begin, offset, value);
    }

    void mpush16(const uint64_t begin, uint64_t & offset, uint16_t value) {
        push_memory_to<uint16_t>(begin, offset, value);
    }

    void mpush32(const uint64_t begin, uint64_t & offset, uint32_t value) {
        push_memory_to<uint32_t>(begin, offset, value);
    }

    void mpush64(const uint64_t begin, uint64_t & offset, uint64_t value) {
        push_memory_to<uint64_t>(begin, offset, value);
    }

    template < typename DataType >
    DataType pop_memory_from(const uint64_t begin, uint64_t & offset)
    {
        DataType result;
        read_memory(begin + offset, (char*)&result, sizeof(DataType));
        offset += sizeof(DataType);
        return result;
    }

    uint8_t mpop8(const uint64_t begin, uint64_t & offset) {
        return pop_memory_from<uint8_t>(begin, offset);
    }

    uint16_t mpop16(const uint64_t begin, uint64_t & offset) {
        return pop_memory_from<uint16_t>(begin, offset);
    }

    uint32_t mpop32(const uint64_t begin, uint64_t & offset) {
        return pop_memory_from<uint32_t>(begin, offset);
    }

    uint64_t mpop64(const uint64_t begin, uint64_t & offset) {
        return pop_memory_from<uint64_t>(begin, offset);
    }

public:
    virtual ~SysdarftCPUMemoryAccess() = default;
    SysdarftCPUMemoryAccess operator=(const SysdarftCPUMemoryAccess&) = delete;
};

#endif //SYSDARFTMEMORY_H
