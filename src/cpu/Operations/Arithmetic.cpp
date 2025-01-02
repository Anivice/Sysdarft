#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::add(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    __uint128_t result = operand1 + operand2;
    uint64_t compliment;

    switch (WidthAndOperands.first) {
    case _8bit_prefix: compliment = 0xFF; break;
    case _16bit_prefix: compliment = 0xFFFF; break;
    case _32bit_prefix: compliment = 0xFFFFFFFF; break;
    case _64bit_prefix: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Invalid operation width");
    }

    if ((result & compliment) != result) { // overflow
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 1;
        FG.Overflow = 1;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }

    WidthAndOperands.second[0].set_val(result & compliment);
}
