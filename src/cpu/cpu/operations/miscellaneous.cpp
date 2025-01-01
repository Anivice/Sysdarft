#include <cpu.h>

std::string processor::__InstructionExecutorType__::bcd_width_str(const uint8_t width)
{
    switch (width) {
    case 0x08: return "8";
    case 0x16: return "16";
    case 0x32: return "32";
    case 0x64: return "64";
    default: throw IllegalInstruction("Unknown width");
    }
}

void processor::__InstructionExecutorType__::nop(const __uint128_t timestamp)
{
    log("[PROCESSOR, ", timestamp, "]: NOP\n");
    // No Operation
}
