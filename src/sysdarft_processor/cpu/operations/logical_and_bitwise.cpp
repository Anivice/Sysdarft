#include <cpu.h>

void processor::__InstructionExecutorType__::and_(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: AND .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum1 & opnum2);

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, 0); // clear OF and CF
}

void processor::__InstructionExecutorType__::or_(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: OR .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum1 | opnum2);

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, 0); // clear OF and CF
}


void processor::__InstructionExecutorType__::xor_(__uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: XOR .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum1 ^ opnum2);

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, 0); // clear OF and CF
}

void processor::__InstructionExecutorType__::not_(__uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: NOT .",
        bcd_width_str(width), "bit ",
        operand.literal, "\n");

    opnum = operand.get<uint64_t>();
    operand = static_cast<uint64_t>(~opnum);

    // clear the register
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.LessThan = 0;
        CPU.Registers.FlagRegister.LargerThan = 0;
        CPU.Registers.FlagRegister.Equal = 0;
    }

    check_overflow(width, 0); // clear OF and CF
}
