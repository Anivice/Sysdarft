#include <cpu.h>

SysdarftRegister processor::real_mode_register_access(const uint8_t RegisterID)
{
    if (RegisterID > core_count - 1) {
        throw IllegalCoreRegisterException("No such core" + std::to_string(RegisterID));
    }

    std::lock_guard<std::mutex> lock(RegisterAccessMutex);
    return *Registers.at(RegisterID);
}

void processor::real_mode_register_store(const SysdarftRegister &reg, const uint8_t RegisterID)
{
    if (RegisterID > core_count - 1) {
        throw IllegalCoreRegisterException("No such core" + std::to_string(RegisterID));
    }

    std::lock_guard<std::mutex> lock(RegisterAccessMutex);
    *Registers[RegisterID] = reg;
}
