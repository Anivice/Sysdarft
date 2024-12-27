#include <cpu.h>

void processor::__InstructionExecutorType__::add(const __uint128_t timestamp)
{
    auto width = CPU.pop<8>();
    __uint128_t compliment = 0, opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]:\tADD ", operand1.literal, ", ", operand2.literal, "\n");

    switch (width) {
    case 0x08: compliment = 0xFF; break;
    case 0x16: compliment = 0xFFFF; break;
    case 0x32: compliment = 0xFFFFFFFF; break;
    case 0x64: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Unknown width");
    }

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    __uint128_t result = opnum1 + opnum2;

    // set flags accordingly
    if (result & compliment == result) {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 0;
        CPU.Registers.FlagRegister.Overflow = 0;
    } else {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 1;
        CPU.Registers.FlagRegister.Overflow = 1;
    }

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::adc(const __uint128_t timestamp)
{
    auto get_cf = [&]()->uint64_t {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        return CPU.Registers.FlagRegister.Carry;
    };

    auto width = CPU.pop<8>();
    __uint128_t compliment = 0, opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]:\tADC ", operand1.literal, ", ", operand2.literal, "\n");

    switch (width) {
    case 0x08: compliment = 0xFF; break;
    case 0x16: compliment = 0xFFFF; break;
    case 0x32: compliment = 0xFFFFFFFF; break;
    case 0x64: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Unknown width");
    }

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    __uint128_t result = opnum1 + opnum2 + get_cf();

    // set flags accordingly
    if (result & compliment == result) {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 0;
        CPU.Registers.FlagRegister.Overflow = 0;
    } else {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 1;
        CPU.Registers.FlagRegister.Overflow = 1;
    }

    operand1 = static_cast<uint64_t>(result);
}
