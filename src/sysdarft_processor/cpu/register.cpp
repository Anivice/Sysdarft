#include <worker.h>
#include <cpu.h>

void processor::initialize_registers()
{
    std::lock_guard lock(RegisterAccessMutex);
    Registers.clear();
    for (int i = 0; i < core_count; i++) {
        Registers.emplace_back(SysdarftRegister());
    }

    Registers[0].InstructionPointer = 0xC1800;
}

SysdarftRegister & SysdarftRegister::operator=(const SysdarftRegister & other)
{
    Registers = other.Registers;
    return *this;
}
