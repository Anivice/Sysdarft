#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::mov(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto src = WidthAndOperands.second[1].get_val();
    WidthAndOperands.second[0].set_val(src);
}
