#include <cpu.h>

void processor::__InstructionExecutorType__::nop(const __uint128_t timestamp)
{
    debug::log("[PROCESSOR, ", timestamp, "]:\tNOP\n");
    // No Operation
}
