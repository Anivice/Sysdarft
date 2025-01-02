#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::add(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    __uint128_t result = operand1 + operand2;
    WidthAndOperands.second[0].set_val(check_overflow(WidthAndOperands.first /* BCD Width */, result));
}
