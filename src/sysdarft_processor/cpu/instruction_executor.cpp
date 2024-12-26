#include <cpu.h>
#include <iostream>
#include <cstdint>
#include <iomanip>

/*
 * Memory Layout:
 * 0x00000 - 0x9FFFF [BOOT CODE]     - 640KB
 * 0xA0000 - 0xC17FF [CONFIGURATION] - 134KB
 *                   [4KB Interruption Table: 512 Interrupts]
 * 0xC1800 - 0xFFFFF [FIRMWARE]      - 250KB
 */

std::mutex interruption_vector_address_table_mutex_;
std::map < uint64_t /* Interruption number */, uint64_t /* Interruption vector address */>
    interruption_vector_address_table;

class __initialize_vector__ {
public:
    __initialize_vector__()
    {
        // Define parameters
        constexpr size_t TOTAL_ENTRIES = 512;

        // Initialize the table
        std::lock_guard lock(interruption_vector_address_table_mutex_);
        for (uint64_t i = 0; i < TOTAL_ENTRIES; i++)
        {
            constexpr uint64_t INCREMENT = 8;
            constexpr uint64_t START_ADDRESS = 0xA0000;
            uint64_t address = START_ADDRESS + i * INCREMENT;
            interruption_vector_address_table.insert(std::make_pair(i, address));
        }
    }
} __initialize_vector_instance__;

// Sample operation function
void processor::operation(__uint128_t timestamp, const uint8_t current_core)
{
    switch (pop<64>(current_core))
    {
        case 0x00: InstructionExecutor.nop(); break;
        default: soft_interruption_ready(current_core, INT_ILLEGAL_INSTRUCTION); break;
    }
}

void processor::soft_interruption_ready(const uint8_t current_core, const uint64_t int_code)
{
    const auto reg = real_mode_register_access(current_core);
}

void processor::__InstructionExecutorType__::nop()
{
    // No Operation
}

void processor::__InstructionExecutorType__::pushall()
{

}
