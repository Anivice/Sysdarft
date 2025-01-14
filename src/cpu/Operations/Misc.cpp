#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::nop(__uint128_t, WidthAndOperandsType &)
{
    // No Operation, do absolutely nothing
}

void SysdarftCPUInstructionExecutor::hlt(__uint128_t, WidthAndOperandsType &)
{
    SystemHalted = true;
}

void SysdarftCPUInstructionExecutor::igni(__uint128_t, WidthAndOperandsType &)
{
    auto fg = SysdarftRegister::load<FlagRegisterType>();
    fg.InterruptionMask = 1;
    SysdarftRegister::store<FlagRegisterType>(fg);
}

void SysdarftCPUInstructionExecutor::alwi(__uint128_t, WidthAndOperandsType &)
{
    auto fg = SysdarftRegister::load<FlagRegisterType>();
    fg.InterruptionMask = 0;
    SysdarftRegister::store<FlagRegisterType>(fg);
}
