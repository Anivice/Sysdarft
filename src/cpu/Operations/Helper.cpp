#include <SysdarftInstructionExec.h>

uint64_t SysdarftCPUInstructionExecutor::check_overflow(const uint8_t BCDWidth, const __uint128_t Value)
{
    uint64_t compliment;

    switch (BCDWidth) {
    case _8bit_prefix: compliment = 0xFF; break;
    case _16bit_prefix: compliment = 0xFFFF; break;
    case _32bit_prefix: compliment = 0xFFFFFFFF; break;
    case _64bit_prefix: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Invalid operation width");
    }

    if ((Value & compliment) != Value) { // overflow
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 1;
        FG.Overflow = 1;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }

    return Value & compliment;
}
