#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::add(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const __uint128_t result = operand1 + operand2;
    WidthAndOperands.second[0].set_val(check_overflow(WidthAndOperands.first /* BCD Width */, result));
}

void SysdarftCPUInstructionExecutor::adc(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto CF = SysdarftRegister::load<FlagRegisterType>().Carry;
    const __uint128_t result = operand1 + operand2 + CF;
    WidthAndOperands.second[0].set_val(check_overflow(WidthAndOperands.first /* BCD Width */, result));
}

void SysdarftCPUInstructionExecutor::sub(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const __uint128_t result = operand1 - operand2;
    WidthAndOperands.second[0].set_val(check_overflow(WidthAndOperands.first /* BCD Width */, result));
}

void SysdarftCPUInstructionExecutor::sbb(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto CF = SysdarftRegister::load<FlagRegisterType>().Carry;
    const __uint128_t result = operand1 - operand2 - CF;
    WidthAndOperands.second[0].set_val(check_overflow(WidthAndOperands.first /* BCD Width */, result));
}

void SysdarftCPUInstructionExecutor::imul(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    uint64_t TargetRegister0;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  TargetRegister0 = SysdarftRegister::load<RegisterType, 0>()             | 0xFFFFFFFFFFFFFF00; break;
    case _16bit_prefix: TargetRegister0 = SysdarftRegister::load<ExtendedRegisterType, 0>()     | 0xFFFFFFFFFFFF0000; break;
    case _32bit_prefix: TargetRegister0 = SysdarftRegister::load<HalfExtendedRegisterType, 0>() | 0xFFFFFFFF00000000; break;
    case _64bit_prefix: TargetRegister0 = SysdarftRegister::load<FullyExtendedRegisterType, 0>(); break;
    default: throw IllegalInstruction("Unknown width");
    }
    const auto operand1 = WidthAndOperands.second[0].get_val();

    const int64_t factor = *(int64_t*)(&operand1);
    const int64_t base = *(int64_t*)(&TargetRegister0);

    const __int128_t signed_result = factor * base;
    uint64_t result;
    if (signed_result > 0) {
        result = check_overflow(WidthAndOperands.first /* BCD Width */, signed_result);
    } else {
        result = check_overflow_signed(WidthAndOperands.first /* BCD Width */, signed_result);
    }

    switch (WidthAndOperands.first) {
    case _8bit_prefix:  SysdarftRegister::store<RegisterType, 0>(result); break;
    case _16bit_prefix: SysdarftRegister::store<ExtendedRegisterType, 0>(result); break;
    case _32bit_prefix: SysdarftRegister::store<HalfExtendedRegisterType, 0>(result); break;
    case _64bit_prefix: SysdarftRegister::store<FullyExtendedRegisterType, 0>(result); break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void SysdarftCPUInstructionExecutor::mul(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    uint64_t TargetRegister0;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  TargetRegister0 = SysdarftRegister::load<RegisterType, 0>(); break;
    case _16bit_prefix: TargetRegister0 = SysdarftRegister::load<ExtendedRegisterType, 0>(); break;
    case _32bit_prefix: TargetRegister0 = SysdarftRegister::load<HalfExtendedRegisterType, 0>(); break;
    case _64bit_prefix: TargetRegister0 = SysdarftRegister::load<FullyExtendedRegisterType, 0>(); break;
    default: throw IllegalInstruction("Unknown width");
    }
    const auto operand1 = WidthAndOperands.second[0].get_val();

    const uint64_t factor = operand1;
    const uint64_t base = TargetRegister0;

    const __uint128_t result = factor * base;
    const auto trimmed_result = check_overflow(WidthAndOperands.first /* BCD Width */, result);

    switch (WidthAndOperands.first) {
    case _8bit_prefix:  SysdarftRegister::store<RegisterType, 0>(trimmed_result); break;
    case _16bit_prefix: SysdarftRegister::store<ExtendedRegisterType, 0>(trimmed_result); break;
    case _32bit_prefix: SysdarftRegister::store<HalfExtendedRegisterType, 0>(trimmed_result); break;
    case _64bit_prefix: SysdarftRegister::store<FullyExtendedRegisterType, 0>(trimmed_result); break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void SysdarftCPUInstructionExecutor::idiv(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    uint64_t TargetRegister0;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  TargetRegister0 = SysdarftRegister::load<RegisterType, 0>()             | 0xFFFFFFFFFFFFFF00; break;
    case _16bit_prefix: TargetRegister0 = SysdarftRegister::load<ExtendedRegisterType, 0>()     | 0xFFFFFFFFFFFF0000; break;
    case _32bit_prefix: TargetRegister0 = SysdarftRegister::load<HalfExtendedRegisterType, 0>() | 0xFFFFFFFF00000000; break;
    case _64bit_prefix: TargetRegister0 = SysdarftRegister::load<FullyExtendedRegisterType, 0>(); break;
    default: throw IllegalInstruction("Unknown width");
    }
    const auto operand1 = WidthAndOperands.second[0].get_val();

    const int64_t factor = *(int64_t*)(&operand1);
    const int64_t base = *(int64_t*)(&TargetRegister0);

    if (factor == 0) {
        do_interruption(0x01);
        return;
    }

    __int128_t quotient = base / factor;
    __int128_t remainder = base % factor;

    if (quotient > 0) {
        quotient = check_overflow(WidthAndOperands.first /* BCD Width */, quotient);
    } else {
        quotient = check_overflow_signed(WidthAndOperands.first /* BCD Width */, quotient);
    }

    if (remainder > 0) {
        remainder = check_overflow(WidthAndOperands.first /* BCD Width */, remainder);
    } else {
        remainder = check_overflow_signed(WidthAndOperands.first /* BCD Width */, remainder);
    }

    switch (WidthAndOperands.first) {
    case _8bit_prefix:
        SysdarftRegister::store<RegisterType, 0>(quotient);
        SysdarftRegister::store<RegisterType, 1>(remainder);
        break;
    case _16bit_prefix:
        SysdarftRegister::store<ExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<ExtendedRegisterType, 1>(remainder);
        break;
    case _32bit_prefix:
        SysdarftRegister::store<HalfExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<HalfExtendedRegisterType, 1>(remainder);
        break;
    case _64bit_prefix:
        SysdarftRegister::store<FullyExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<FullyExtendedRegisterType, 1>(remainder);
        break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void SysdarftCPUInstructionExecutor::div(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    uint64_t TargetRegister0;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  TargetRegister0 = SysdarftRegister::load<RegisterType, 0>()             | 0xFFFFFFFFFFFFFF00; break;
    case _16bit_prefix: TargetRegister0 = SysdarftRegister::load<ExtendedRegisterType, 0>()     | 0xFFFFFFFFFFFF0000; break;
    case _32bit_prefix: TargetRegister0 = SysdarftRegister::load<HalfExtendedRegisterType, 0>() | 0xFFFFFFFF00000000; break;
    case _64bit_prefix: TargetRegister0 = SysdarftRegister::load<FullyExtendedRegisterType, 0>(); break;
    default: throw IllegalInstruction("Unknown width");
    }
    const auto operand1 = WidthAndOperands.second[0].get_val();

    const uint64_t factor = operand1;
    const uint64_t base = TargetRegister0;

    if (factor == 0) {
        do_interruption(0x01);
        return;
    }

    __uint128_t quotient = base / factor;
    __uint128_t remainder = base % factor;

    quotient = check_overflow(WidthAndOperands.first /* BCD Width */, quotient);
    remainder = check_overflow(WidthAndOperands.first /* BCD Width */, remainder);

    switch (WidthAndOperands.first) {
    case _8bit_prefix:
        SysdarftRegister::store<RegisterType, 0>(quotient);
        SysdarftRegister::store<RegisterType, 1>(remainder);
        break;
    case _16bit_prefix:
        SysdarftRegister::store<ExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<ExtendedRegisterType, 1>(remainder);
        break;
    case _32bit_prefix:
        SysdarftRegister::store<HalfExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<HalfExtendedRegisterType, 1>(remainder);
        break;
    case _64bit_prefix:
        SysdarftRegister::store<FullyExtendedRegisterType, 0>(quotient);
        SysdarftRegister::store<FullyExtendedRegisterType, 1>(remainder);
        break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void SysdarftCPUInstructionExecutor::neg(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    auto operand1 = WidthAndOperands.second[0].get_val();
    operand1 = -operand1;
    WidthAndOperands.second[0].set_val(operand1);
    check_overflow(_8bit_prefix, 0); // clear arithmetic flags
}

void SysdarftCPUInstructionExecutor::cmp(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();

    auto FG = SysdarftRegister::load<FlagRegisterType>();
    check_overflow(_8bit_prefix, 0); // clear arithmetic flags

    if (operand1 > operand2) {
        FG.LargerThan = 1;
    } else if (operand1 == operand2) {
        FG.Equal = 1;
    } else if (operand1 < operand2) {
        FG.LessThan = 1;
    }

    SysdarftRegister::store<FlagRegisterType>(FG);
}
