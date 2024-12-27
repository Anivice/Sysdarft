#include <worker.h>
#include <cpu.h>

void processor::initialize_registers()
{
    std::lock_guard lock(RegisterAccessMutex);
    Registers.InstructionPointer = BIOS_START;
}
