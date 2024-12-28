#include <cpu.h>

void processor::__InstructionExecutorType__::check_overflow(const uint8_t bcd_width, const __uint128_t val)
{
    __uint128_t compliment = 0;
    switch (bcd_width) {
    case 0x08: compliment = 0xFF; break;
    case 0x16: compliment = 0xFFFF; break;
    case 0x32: compliment = 0xFFFFFFFF; break;
    case 0x64: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Unknown width");
    }

    // set flags accordingly
    if (val & compliment == val) {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 0;
        CPU.Registers.FlagRegister.Overflow = 0;
    } else {
        std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = 1;
        CPU.Registers.FlagRegister.Overflow = 1;
    }
}

void processor::__InstructionExecutorType__::add(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: ADD ", operand1.literal, ", ", operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    const __uint128_t result = opnum1 + opnum2;

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
    debug::log("[PROCESSOR, ", timestamp, "]: ADC ", operand1.literal, ", ", operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 + opnum2 + get_cf();

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::sub(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: SUB ", operand1.literal, ", ", operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 - opnum2;

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
    debug::log("[PROCESSOR, ", timestamp, "]: SBB ", operand1.literal, ", ", operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    const __uint128_t result = opnum1 - opnum2 - get_cf();

    check_overflow(width, result);

    operand1 = static_cast<uint64_t>(result);
}

void processor::__InstructionExecutorType__::imul(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand1 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: IMUL ", operand1.literal, "\n");

    opnum = operand1.get<uint64_t>();
    uint64_t TargetRegister0 = 0;

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: TargetRegister0 = CPU.Registers.Register0; break;
        case 0x16: TargetRegister0 = CPU.Registers.ExtendedRegister0; break;
        case 0x32: TargetRegister0 = CPU.Registers.HalfExtendedRegister0; break;
        case 0x64: TargetRegister0 = CPU.Registers.FullyExtendedRegister0; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }

    const int64_t factor = *(int64_t*)(&opnum);
    const int64_t base = *(int64_t*)(&TargetRegister0);
    __int128_t signed_result = factor * base;

    check_overflow(width, *(__uint128_t*)&signed_result);

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        switch (width) {
        case 0x08: CPU.Registers.Register0                     = (*(__uint128_t*)&signed_result) & 0xFF; break;
        case 0x16: CPU.Registers.ExtendedRegister0      = (*(__uint128_t*)&signed_result) & 0xFFFF; break;
        case 0x32: CPU.Registers.HalfExtendedRegister0  = (*(__uint128_t*)&signed_result) & 0xFFFFFFFF; break;
        case 0x64: CPU.Registers.FullyExtendedRegister0 = (*(__uint128_t*)&signed_result) & 0xFFFFFFFFFFFFFFFF; break;
        default: throw IllegalInstruction("Unknown width");
        }
    }
}
