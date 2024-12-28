#include <cpu.h>

void processor::__InstructionExecutorType__::pushall(__uint128_t timestamp)
{
}

void processor::__InstructionExecutorType__::mov(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: MOV .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum = operand2.get<uint64_t>();

    check_overflow(width, opnum);

    operand1 = static_cast<uint64_t>(opnum);
}
