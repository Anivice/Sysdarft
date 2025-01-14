#include <SysdarftMemory.h>
#include <cstring>
#include <mutex>

SysdarftCPUMemoryAccess::SysdarftCPUMemoryAccess(const uint64_t totalMemory)
{
    std::lock_guard<std::mutex> lock(MemoryAccessMutex);
    Memory.clear();
    TotalMemory = totalMemory;

    if (totalMemory == 0) {
        throw SysdarftBaseError("Total memory is zero");
    }

    // Fill the vector with (TotalMemory / BLOCK_SIZE) blocks
    const uint64_t numBlocks = TotalMemory / BLOCK_SIZE;
    Memory.reserve(numBlocks);

    for (uint64_t i = 0; i < numBlocks; i++) {
        Memory.emplace_back();
    }
}

void SysdarftCPUMemoryAccess::read_memory(const uint64_t address, char* _dest, const uint64_t size)
{
    std::lock_guard<std::mutex> lock(MemoryAccessMutex);

    // Basic range check
    if (address > TotalMemory || (address + size) > TotalMemory) {
        throw IllegalMemoryAccessException("Memory access out of bounds");
    }

    const uint64_t page_address = address / BLOCK_SIZE;
    const uint64_t page_offset = address % BLOCK_SIZE;

    if (size <= (BLOCK_SIZE - page_offset)) {
        // Entire read fits in one block
        std::memcpy(_dest, Memory[page_address].data() + page_offset, size);
    } else {
        // Multi-page read
        const uint64_t leftover_on_first_page = BLOCK_SIZE - page_offset;
        const uint64_t rest_size = size - leftover_on_first_page;
        const uint64_t full_pages = rest_size / BLOCK_SIZE;
        const uint64_t leftover_on_last_page = rest_size % BLOCK_SIZE;

        uint64_t current_offset = 0;

        // 1. Read partial chunk from the first page
        std::memcpy(_dest, Memory[page_address].data() + page_offset, leftover_on_first_page);
        current_offset += leftover_on_first_page;

        // 2. Read each full page
        for (uint64_t i = 1; i <= full_pages; i++) {
            std::memcpy(_dest + current_offset,
                        Memory[page_address + i].data(),
                        BLOCK_SIZE);
            current_offset += BLOCK_SIZE;
        }

        // 3. Read leftover chunk on the next page
        if (leftover_on_last_page > 0) {
            std::memcpy(_dest + current_offset,
                        Memory[page_address + full_pages + 1].data(),
                        leftover_on_last_page);
        }
    }
}

void SysdarftCPUMemoryAccess::write_memory(const uint64_t address, const char* _source, const uint64_t size)
{
    std::lock_guard<std::mutex> lock(MemoryAccessMutex);

    // Basic range check
    if (address > TotalMemory || (address + size) > TotalMemory) {
        throw IllegalMemoryAccessException("Memory access out of bounds");
    }

    const uint64_t page_address = address / BLOCK_SIZE;
    const uint64_t page_offset = address % BLOCK_SIZE;

    // Helper lambda to copy data into our Memory blocks with bounds-check
    auto copy_n = [](std::array<uint8_t, BLOCK_SIZE>& destBlock,
                     const uint64_t offset,
                     const char* src,
                     const uint64_t count) -> void
    {
        if (offset + count > BLOCK_SIZE) {
            throw IllegalMemoryAccessException("Memory write out of bounds");
        }
        // Copy byte by byte (could use memcpy if you prefer)
        for (uint64_t i = 0; i < count; i++) {
            destBlock[offset + i] = static_cast<uint8_t>(src[i]);
        }
    };

    if (size <= (BLOCK_SIZE - page_offset)) {
        // Entire write fits in one block
        copy_n(Memory[page_address], page_offset, _source, size);
    } else {
        // Multi-page write
        const uint64_t leftover_on_first_page = BLOCK_SIZE - page_offset;
        const uint64_t rest_size = size - leftover_on_first_page;
        const uint64_t full_pages = rest_size / BLOCK_SIZE;
        const uint64_t leftover_on_last_page = rest_size % BLOCK_SIZE;

        uint64_t current_offset = 0;

        // 1. Write partial chunk to the first page
        copy_n(Memory[page_address], page_offset, _source, leftover_on_first_page);
        current_offset += leftover_on_first_page;

        // 2. Write full pages
        for (uint64_t i = 1; i <= full_pages; i++) {
            copy_n(Memory[page_address + i], 0, _source + current_offset, BLOCK_SIZE);
            current_offset += BLOCK_SIZE;
        }

        // 3. Write leftover chunk on the next page
        if (leftover_on_last_page > 0) {
            copy_n(Memory[page_address + full_pages + 1], 0,
                   _source + current_offset, leftover_on_last_page);
        }
    }
}
