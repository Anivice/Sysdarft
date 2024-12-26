#include <worker.h>
#include <cpu.h>

void processor::initialize_registers()
{
    std::lock_guard lock(RegisterAccessMutex);
    Registers.clear();
    Registers.reserve(core_count);
    for (int i = 0; i < core_count; i++) {
        auto reg = std::make_unique<SysdarftRegister>();
        Registers.emplace_back(std::move(reg)); // Move the unique_ptr into the vector
    }

    if (!Registers.empty()) {
        Registers[0]->InstructionPointer = 0xC1800;
    }
}

SysdarftRegister & SysdarftRegister::operator=(const SysdarftRegister & other)
{
    if (this != &other) {
        // Perform a deep copy of all necessary members
        this->Registers = other.Registers; // Ensure Registers is a proper deep copy
        // Copy other members as needed
    }
    return *this;
}
