#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::add(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    auto operand1 = WidthAndOperands.second[0].get_val();
    auto operand2 = WidthAndOperands.second[1].get_val();
    __uint128_t result = operand1 + operand2;
    if ((result & 0xFFFFFFFFFFFFFFFF) != result) { // overflow
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 1;
        FG.Overflow = 1;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }
}
