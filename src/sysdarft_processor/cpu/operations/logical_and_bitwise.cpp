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
}

void processor::__InstructionExecutorType__::xor_(const __uint128_t timestamp)
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
}

void processor::__InstructionExecutorType__::not_(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum = 0;
    auto operand = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: NOT .",
        bcd_width_str(width), "bit ",
        operand.literal, "\n");

    opnum = operand.get<uint64_t>();
    operand = static_cast<uint64_t>(~opnum);
}

void processor::__InstructionExecutorType__::shl(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: SHL .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum1 << opnum2);
}

void processor::__InstructionExecutorType__::shr(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: SHR .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();
    operand1 = static_cast<uint64_t>(opnum1 >> opnum2);
}

template <unsigned SIZE>
requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type rotate_left(typename size_to_uint<SIZE>::type value, uint64_t n)
{
    constexpr unsigned bits = SIZE;
    n %= bits; // Ensure n is within [0, bits-1]
    if (n == 0) return value;
    return (value << n) | (value >> (bits - n));
}

// Function to rotate bits to the right
template <unsigned SIZE>
requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type rotate_right(typename size_to_uint<SIZE>::type value, uint64_t n)
{
    constexpr unsigned bits = SIZE;
    n %= bits; // Ensure n is within [0, bits-1]
    if (n == 0) return value;
    return (value >> n) | (value << (bits - n));
}

void processor::__InstructionExecutorType__::rol(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: ROL .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    switch (width) {
    case 0x08: operand1 = rotate_left<8>(static_cast<uint8_t>(opnum1), opnum2); break;
    case 0x16: operand1 = rotate_left<16>(static_cast<uint16_t>(opnum1), opnum2); break;
    case 0x32: operand1 = rotate_left<32>(static_cast<uint32_t>(opnum1), opnum2); break;
    case 0x64: operand1 = rotate_left<64>(static_cast<uint64_t>(opnum1), opnum2); break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void processor::__InstructionExecutorType__::ror(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: ROR .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    switch (width) {
    case 0x08: operand1 = rotate_right<8>(static_cast<uint8_t>(opnum1), opnum2); break;
    case 0x16: operand1 = rotate_right<16>(static_cast<uint16_t>(opnum1), opnum2); break;
    case 0x32: operand1 = rotate_right<32>(static_cast<uint32_t>(opnum1), opnum2); break;
    case 0x64: operand1 = rotate_right<64>(static_cast<uint64_t>(opnum1), opnum2); break;
    default: throw IllegalInstruction("Unknown width");
    }
}

// Function to rotate bits to the left with an extra_bit
template <unsigned SIZE>
requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type rotate_left_through_bit(
    typename size_to_uint<SIZE>::type value, uint64_t n, int &extra_bit)
{
    using uint_type = typename size_to_uint<SIZE>::type; // Type alias for convenience
    constexpr unsigned bits = SIZE;
    if (bits == 0) return value; // Avoid division by zero

    n %= bits; // Ensure n is within [0, bits-1]
    if (n == 0) return value;

    // Mask to extract n bits
    uint_type mask_n = (n == bits) ?
        static_cast<uint_type>(-1) : ((static_cast<uint_type>(1) << n) - 1);

    // Extract incoming bits from extra_bit (lower n bits)
    uint_type incoming_bits = static_cast<uint_type>(extra_bit) & mask_n;

    // Extract bits that will be rotated out (higher n bits of value)
    uint_type bits_out = (value >> (bits - n)) & mask_n;

    // Update extra_bit with the bits rotated out
    extra_bit = static_cast<int>(bits_out);

    // Perform the rotation
    uint_type rotated_value = (value << n) | incoming_bits;

    return rotated_value;
}

// Function to rotate bits to the right with an extra_bit
template <unsigned SIZE>
requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type rotate_right_through_bit(
    typename size_to_uint<SIZE>::type value, uint64_t n, int &extra_bit)
{
    using uint_type = typename size_to_uint<SIZE>::type; // Type alias for convenience
    constexpr unsigned bits = SIZE;
    if (bits == 0) return value; // Avoid division by zero

    n %= bits; // Ensure n is within [0, bits-1]
    if (n == 0) return value;

    // Mask to extract n bits
    uint_type mask_n = (n == bits) ?
        static_cast<uint_type>(-1) : ((static_cast<uint_type>(1) << n) - 1);

    // Extract incoming bits from extra_bit (lower n bits)
    uint_type incoming_bits = static_cast<uint_type>(extra_bit) & mask_n;

    // Extract bits that will be rotated out (lower n bits of value)
    uint_type bits_out = value & mask_n;

    // Update extra_bit with the bits rotated out
    extra_bit = static_cast<int>(bits_out);

    // Perform the rotation
    uint_type rotated_value = (value >> n) | (incoming_bits << (bits - n));

    return rotated_value;
}

void processor::__InstructionExecutorType__::rcl(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: RCL .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    int eb;
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        eb = CPU.Registers.FlagRegister.Carry;
    }

    switch (width) {
    case 0x08: operand1 = rotate_left_through_bit<8>(static_cast<uint8_t>(opnum1), opnum2, eb); break;
    case 0x16: operand1 = rotate_left_through_bit<16>(static_cast<uint16_t>(opnum1), opnum2, eb); break;
    case 0x32: operand1 = rotate_left_through_bit<32>(static_cast<uint32_t>(opnum1), opnum2, eb); break;
    case 0x64: operand1 = rotate_left_through_bit<64>(static_cast<uint64_t>(opnum1), opnum2, eb); break;
    default: throw IllegalInstruction("Unknown width");
    }

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = eb & 0x01;
    }
}

void processor::__InstructionExecutorType__::rcr(const __uint128_t timestamp)
{
    const auto width = CPU.pop<8>();
    __uint128_t opnum1 = 0, opnum2 = 0;
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR, ", timestamp, "]: RCL .",
        bcd_width_str(width), "bit ",
        operand1.literal, ", ",
        operand2.literal, "\n");

    opnum1 = operand1.get<uint64_t>();
    opnum2 = operand2.get<uint64_t>();

    int eb;
    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        eb = CPU.Registers.FlagRegister.Carry;
    }

    switch (width) {
    case 0x08: operand1 = rotate_right_through_bit<8>(static_cast<uint8_t>(opnum1), opnum2, eb); break;
    case 0x16: operand1 = rotate_right_through_bit<16>(static_cast<uint16_t>(opnum1), opnum2, eb); break;
    case 0x32: operand1 = rotate_right_through_bit<32>(static_cast<uint32_t>(opnum1), opnum2, eb); break;
    case 0x64: operand1 = rotate_right_through_bit<64>(static_cast<uint64_t>(opnum1), opnum2, eb); break;
    default: throw IllegalInstruction("Unknown width");
    }

    {
        std::lock_guard lock(CPU.RegisterAccessMutex);
        CPU.Registers.FlagRegister.Carry = eb & 0x01;
    }
}
