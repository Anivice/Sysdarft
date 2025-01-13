#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::nop(__uint128_t, WidthAndOperandsType &)
{
    // No Operation, do absolutely nothing
}

void SysdarftCPUInstructionExecutor::hlt(__uint128_t, WidthAndOperandsType &)
{
    SystemHalted = true;
}
