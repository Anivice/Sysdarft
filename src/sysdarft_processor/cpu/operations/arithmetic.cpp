#include <cpu.h>

void processor::__InstructionExecutorType__::add(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: ADD .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    const __uint128_t result = opnum1 + opnum2;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::adc(const __uint128_t timestamp)
{
    auto get_cf = [&]()->uint64_t {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        return CPU.Registers.FlagRegister.Carry;
    };

    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: ADC .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 + opnum2 + get_cf();

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::sub(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: SUB .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 - opnum2;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::sbb(const __uint128_t timestamp)
{
    auto get_cf = [&]()->uint64_t {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        return CPU.Registers.FlagRegister.Carry;
    };

    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: SBB .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 - opnum2 - get_cf();

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::imul(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: IMUL .",
        bcd_width_str(width), "bit ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    uint64_t TargetRegister0;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: TargetRegister0 = CPU.Registers.Register0             | 0xFFFFFFFFFFFFFF00; break;
        case 0x16: TargetRegister0 = CPU.Registers.ExtendedRegister0     | 0xFFFFFFFFFFFF0000; break;
        case 0x32: TargetRegister0 = CPU.Registers.HalfExtendedRegister0 | 0xFFFFFFFF00000000; break;
        case 0x64: TargetRegister0 = CPU.Registers.FullyExtendedRegister0; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    const int64_t factor = *(int64_t*)(&opnum);
    const int64_t base = *(int64_t*)(&TargetRegister0);
    __int128_t signed_result = factor * base;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, *(__uint128_t*)&signed_result);

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register0              = (*(__uint128_t*)&signed_result) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister0      = (*(__uint128_t*)&signed_result) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister0  = (*(__uint128_t*)&signed_result) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister0 = (*(__uint128_t*)&signed_result) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}

void processor::__InstructionExecutorType__::mul(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: MUL .",
        bcd_width_str(width), "bit ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    __uint128_t TargetRegister0;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: TargetRegister0 = CPU.Registers.Register0             ; break;
        case 0x16: TargetRegister0 = CPU.Registers.ExtendedRegister0     ; break;
        case 0x32: TargetRegister0 = CPU.Registers.HalfExtendedRegister0 ; break;
        case 0x64: TargetRegister0 = CPU.Registers.FullyExtendedRegister0; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    const __uint128_t result = TargetRegister0 * opnum;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, result);

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register0              = (*(__uint128_t*)&result) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister0      = (*(__uint128_t*)&result) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister0  = (*(__uint128_t*)&result) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister0 = (*(__uint128_t*)&result) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}

void processor::__InstructionExecutorType__::idiv(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: IDIV .",
        bcd_width_str(width), "bit ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    uint64_t TargetRegister0;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: TargetRegister0 = CPU.Registers.Register0             | 0xFFFFFFFFFFFFFF00; break;
        case 0x16: TargetRegister0 = CPU.Registers.ExtendedRegister0     | 0xFFFFFFFFFFFF0000; break;
        case 0x32: TargetRegister0 = CPU.Registers.HalfExtendedRegister0 | 0xFFFFFFFF00000000; break;
        case 0x64: TargetRegister0 = CPU.Registers.FullyExtendedRegister0; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    const int64_t factor = *(int64_t*)(&opnum);
    const int64_t base = *(int64_t*)(&TargetRegister0);
    __int128_t quotient = base / factor;
    __int128_t remainder = base % factor;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, *(__uint128_t*)&quotient);
    check_overflow(width, *(__uint128_t*)&remainder);

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register0              = (*(__uint128_t*)&quotient) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister0      = (*(__uint128_t*)&quotient) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister0  = (*(__uint128_t*)&quotient) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister0 = (*(__uint128_t*)&quotient) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register1              = (*(__uint128_t*)&remainder) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister1      = (*(__uint128_t*)&remainder) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister1  = (*(__uint128_t*)&remainder) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister1 = (*(__uint128_t*)&remainder) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}

void processor::__InstructionExecutorType__::div(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: DIV .",
        bcd_width_str(width), "bit ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    uint64_t TargetRegister0;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: TargetRegister0 = CPU.Registers.Register0             ; break;
        case 0x16: TargetRegister0 = CPU.Registers.ExtendedRegister0     ; break;
        case 0x32: TargetRegister0 = CPU.Registers.HalfExtendedRegister0 ; break;
        case 0x64: TargetRegister0 = CPU.Registers.FullyExtendedRegister0; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    const uint64_t factor = opnum;
    const uint64_t base = TargetRegister0;
    __int128_t quotient = base / factor;
    __int128_t remainder = base % factor;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, *(__uint128_t*)&quotient);
    check_overflow(width, *(__uint128_t*)&remainder);

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register0              = (*(__uint128_t*)&quotient) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister0      = (*(__uint128_t*)&quotient) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister0  = (*(__uint128_t*)&quotient) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister0 = (*(__uint128_t*)&quotient) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register1              = (*(__uint128_t*)&remainder) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister1      = (*(__uint128_t*)&remainder) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister1  = (*(__uint128_t*)&remainder) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister1 = (*(__uint128_t*)&remainder) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}

void processor::__InstructionExecutorType__::neg(__uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: NEG .",
        bcd_width_str(width), "bit ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    opnum = -opnum;

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, opnum);

    operand1 = (uint64_t)(opnum & 0xFFFFFFFFFFFFFFFF);
}

void processor::__InstructionExecutorType__::cmp(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: CMP .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    if (opnum1 > opnum2) {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LargerThan = 1;
    } else if (opnum1 < opnum2) {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 1;
    } else if (opnum1 == opnum2) {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Equal = 1;
    }

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, 0); // clear OF and CF
}
