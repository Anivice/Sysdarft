#include <cpu.h>

void processor::__InstructionExecutorType__::check_overflow(const uint8_t bcd_width, const __uint128_t val)
{
    __uint128_t compliment = 0;
    switch (bcd_width) {
    case 0x08: compliment = 0xFF; break;
    case 0x16: compliment = 0xFFFF; break;
    case 0x32: compliment = 0xFFFFFFFF; break;
    case 0x64: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Unknown width");
    }

    // set flags accordingly
    if ((val & compliment) == val) {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 0;
        CPU.Registers.FlagRegister.Overflow = 0;
    } else {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 1;
        CPU.Registers.FlagRegister.Overflow = 1;
    }
}
