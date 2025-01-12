#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::jmp(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const uint64_t addr_base = WidthAndOperands.second[0].get_val();
    const uint64_t ip = WidthAndOperands.second[1].get_val();
    SysdarftRegister::store<CodeBaseType>(addr_base);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::call(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto ori_cb = SysdarftRegister::load<CodeBaseType>();
    const auto ori_ip = SysdarftRegister::load<InstructionPointerType>();

    push_stack(ori_cb);
    push_stack(ori_ip);

    const uint64_t addr_base = WidthAndOperands.second[0].get_val();
    const uint64_t ip = WidthAndOperands.second[1].get_val();
    SysdarftRegister::store<CodeBaseType>(addr_base);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::ret(__uint128_t, WidthAndOperandsType &)
{
    const auto ip = pop_stack<uint64_t>();
    const auto cb = pop_stack<uint64_t>();
    SysdarftRegister::store<CodeBaseType>(cb);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::je(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jne(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (!SysdarftRegister::load<FlagRegisterType>().Equal)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jb(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().LargerThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jl(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().LessThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jbe(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal || SysdarftRegister::load<FlagRegisterType>().LargerThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jle(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal || SysdarftRegister::load<FlagRegisterType>().LessThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::int_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const uint64_t code = WidthAndOperands.second[0].get_val();
    SysdarftCPUInterruption::do_interruption(code);
}

void SysdarftCPUInstructionExecutor::int3(__uint128_t, WidthAndOperandsType &)
{
    SysdarftCPUInterruption::do_interruption(0x03);
}

void SysdarftCPUInstructionExecutor::iret(__uint128_t, WidthAndOperandsType &)
{
    SysdarftCPUInterruption::do_iret();
}
