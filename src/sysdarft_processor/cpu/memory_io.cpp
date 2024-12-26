#include <cpu.h>

void processor::initialize_memory()
{
    std::lock_guard lock(MemoryAccessMutex);
    Memory.clear();
    for (uint64_t i = 0; i < TotalMemory / PAGE_SIZE; i++) {
        Memory.emplace_back(std::array < unsigned char, PAGE_SIZE >());
    }
}

void processor::get_memory(const uint64_t address, char * _dest, const uint64_t size) const
{
    const uint64_t page_address = address / PAGE_SIZE;
    const uint64_t page_offset = address % PAGE_SIZE;

    if (address > TotalMemory /* Starting point out of boundary */
        || address + size > TotalMemory /* End point out of boundary */)
    {
        throw IllegalMemoryAccessException("Memory access out of bounds");
    }

    if (size < PAGE_SIZE) {
        std::memcpy(_dest, Memory[page_address].data(), size);
    } else {
        const uint64_t leftover_on_first_page = PAGE_SIZE - page_offset;
        const uint64_t rest_size_without_leftover = size - leftover_on_first_page;
        const uint64_t full_pages = rest_size_without_leftover / PAGE_SIZE;
        const uint64_t leftover_on_last_page = rest_size_without_leftover % PAGE_SIZE;
        uint64_t current_offset = 0;

        // 1. Read first page
        std::memcpy(_dest, Memory[page_address].data() + page_offset, leftover_on_first_page);
        current_offset += leftover_on_first_page;

        // 2. Read full pages
        for (uint64_t i = 1; i <= full_pages; i++)
        {
            std::memcpy(_dest + current_offset, Memory[page_address + i].data(), PAGE_SIZE);
            current_offset += PAGE_SIZE;
        }

        // 3. Read last page
        std::memcpy(_dest + current_offset, Memory[page_address + full_pages].data(), leftover_on_last_page);
    }
}

void processor::write_memory(const uint64_t address, const char* _source, const uint64_t size)
{
    const uint64_t page_address = address / PAGE_SIZE;
    const uint64_t page_offset = address % PAGE_SIZE;

    if (address > TotalMemory /* Starting point out of boundary */
        || address + size > TotalMemory /* End point out of boundary */)
    {
        throw IllegalMemoryAccessException("Memory access out of bounds");
    }

    if (size < PAGE_SIZE) {
        std::memcpy(Memory[page_address].data(), _source, size);
    } else {
        const uint64_t leftover_on_first_page = PAGE_SIZE - page_offset;
        const uint64_t rest_size_without_leftover = size - leftover_on_first_page;
        const uint64_t full_pages = rest_size_without_leftover / PAGE_SIZE;
        const uint64_t leftover_on_last_page = rest_size_without_leftover % PAGE_SIZE;
        uint64_t current_offset = 0;

        // 1. Write to the first page
        std::memcpy(Memory[page_address].data() + page_offset, _source, leftover_on_first_page);
        current_offset += leftover_on_first_page;

        // 2. Write full pages
        for (uint64_t i = 1; i <= full_pages; i++)
        {
            std::memcpy(Memory[page_address + i].data(), _source + current_offset, PAGE_SIZE);
            current_offset += PAGE_SIZE;
        }

        // 3. Write last page
        std::memcpy(Memory[page_address + full_pages].data(),
            _source + current_offset, leftover_on_last_page);
    }
}
