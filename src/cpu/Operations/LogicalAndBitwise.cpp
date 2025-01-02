#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::and_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto result = operand1 & operand2;
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::or_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto result = operand1 | operand2;
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::xor_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto result = operand1 ^ operand2;
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::not_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto result = ~operand1;
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::shl(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto result = operand1 << operand2;
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::shr(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    const auto result = operand1 >> operand2;
    WidthAndOperands.second[0].set_val(result);
}


// Type trait to map SIZE to corresponding unsigned integer type
template <size_t SIZE>
struct size_to_uint;

// Specializations for supported sizes
template <>
struct size_to_uint<8> {
    using type = uint8_t;
};

template <>
struct size_to_uint<16> {
    using type = uint16_t;
};

template <>
struct size_to_uint<32> {
    using type = uint32_t;
};

template <>
struct size_to_uint<64> {
    using type = uint64_t;
};

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

void SysdarftCPUInstructionExecutor::rol(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = rotate_left<8> (static_cast<uint8_t>(operand1), operand2); break;
    case _16bit_prefix: result = rotate_left<16>(static_cast<uint16_t>(operand1), operand2); break;
    case _32bit_prefix: result = rotate_left<32>(static_cast<uint32_t>(operand1), operand2); break;
    case _64bit_prefix: result = rotate_left<64>(static_cast<uint64_t>(operand1), operand2); break;
    default: throw IllegalInstruction("Unknown width");
    }

    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::ror(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = rotate_right<8> (static_cast<uint8_t>(operand1), operand2); break;
    case _16bit_prefix: result = rotate_right<16>(static_cast<uint16_t>(operand1), operand2); break;
    case _32bit_prefix: result = rotate_right<32>(static_cast<uint32_t>(operand1), operand2); break;
    case _64bit_prefix: result = rotate_right<64>(static_cast<uint64_t>(operand1), operand2); break;
    default: throw IllegalInstruction("Unknown width");
    }

    WidthAndOperands.second[0].set_val(result);
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

void SysdarftCPUInstructionExecutor::rcl(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    int cf = SysdarftRegister::load<FlagRegisterType>().Carry;
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = rotate_left_through_bit<8> (static_cast<uint8_t>(operand1), operand2, cf); break;
    case _16bit_prefix: result = rotate_left_through_bit<16>(static_cast<uint16_t>(operand1), operand2, cf); break;
    case _32bit_prefix: result = rotate_left_through_bit<32>(static_cast<uint32_t>(operand1), operand2, cf); break;
    case _64bit_prefix: result = rotate_left_through_bit<64>(operand1, operand2, cf); break;
    default: throw IllegalInstruction("Unknown width");
    }

    auto FG = SysdarftRegister::load<FlagRegisterType>();
    FG.Carry = cf;
    SysdarftRegister::store<FlagRegisterType>(FG);
    WidthAndOperands.second[0].set_val(result);
}

void SysdarftCPUInstructionExecutor::rcr(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    int cf = SysdarftRegister::load<FlagRegisterType>().Carry;
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = rotate_right_through_bit<8> (static_cast<uint8_t>(operand1), operand2, cf); break;
    case _16bit_prefix: result = rotate_right_through_bit<16>(static_cast<uint16_t>(operand1), operand2, cf); break;
    case _32bit_prefix: result = rotate_right_through_bit<32>(static_cast<uint32_t>(operand1), operand2, cf); break;
    case _64bit_prefix: result = rotate_right_through_bit<64>(operand1, operand2, cf); break;
    default: throw IllegalInstruction("Unknown width");
    }

    auto FG = SysdarftRegister::load<FlagRegisterType>();
    FG.Carry = cf;
    SysdarftRegister::store<FlagRegisterType>(FG);
    WidthAndOperands.second[0].set_val(result);
}
