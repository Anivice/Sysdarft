/* SysdarftMemory.h
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

#ifndef SYSDARFTMEMORY_H
#define SYSDARFTMEMORY_H

#include <SysdarftDebug.h>
#include <array>

/*
 * Memory Layout:
 * Lower 1 MB Firmware Data
 * 0x00000 - 0x9FFFF [BOOT CODE]     - 640KB
 * 0xA0000 - 0xC17FF [CONFIGURATION] - 134KB
 *                    - 0xA0000 - 0xA0FFF [4KB Interruption Table: 256 Interrupts]
 *                    - 0xB8000 - 0xB87CF [2000 Bytes, 80x25 Video Space]
 * 0xC1800 - 0xFFFFF [FIRMWARE]      - 250KB
 */
#define BOOT_LOADER_START   (0x00000)
#define BOOT_LOADER_END     (0x9FFFF)
#define BOOT_LOADER_SIZE    (BOOT_LOADER_END - BOOT_LOADER_START + 1)
#define INTERRUPTION_VECTOR (0xA0000)
#define INTERRUPTION_VEC_ED (0xA0FFF)
#define INTERRUPTION_VEC_LN (INTERRUPTION_VEC_ED - INTERRUPTION_VECTOR + 1)
#define MAX_INTERRUPTION_ENTRY (INTERRUPTION_VEC_LN / (sizeof(uint64_t) * 2))
#define VIDEO_MEMORY_START  (0xB8000)
#define VIDEO_MEMORY_END    (0xB87CF)
#define VIDEO_MEMORY_SIZE   (VIDEO_MEMORY_END - VIDEO_MEMORY_START + 1)
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

class StackOverflow final : public SysdarftBaseError {
public:
    StackOverflow() : SysdarftBaseError("Stack overflow") { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUMemoryAccess
{
public:
    void read_memory(uint64_t address, char * _dest, uint64_t size);
    void write_memory(uint64_t address, const char* _source, uint64_t size);

protected:
    std::mutex MemoryAccessMutex;
    std::vector < std::array < uint8_t, BLOCK_SIZE > > Memory;
    std::atomic<uint64_t> TotalMemory = 0; // 32MB Memory

    explicit SysdarftCPUMemoryAccess(uint64_t totalMemory);

    template < typename DataType >
    void push_memory_to(const uint64_t begin, uint64_t & offset, const DataType & val)
    {
        if (offset < sizeof(DataType)) {
            throw StackOverflow();
        }

        offset -= sizeof(DataType);
        try {
            write_memory(begin + offset, (char*)&val, sizeof(DataType));
        } catch (IllegalMemoryAccessException &) {
            throw StackOverflow();
        }
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

        try {
            read_memory(begin + offset, (char*)&result, sizeof(DataType));
        } catch (IllegalMemoryAccessException &) {
            throw StackOverflow();
        }

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
