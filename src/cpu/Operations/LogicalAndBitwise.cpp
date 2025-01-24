/* LogicalAndBitwise.cpp
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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

// ---------------------------------------------------------------------------
// 2) RCL: Rotate through carry LEFT by 'n' bits
//    - value: the SIZE-bit operand
//    - n: number of bits to rotate
//    - cf: the carry flag (single bit, 0 or 1) passed by reference
// ---------------------------------------------------------------------------
template <unsigned SIZE>
  requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type
rcl(typename size_to_uint<SIZE>::type value, unsigned n, bool &cf)
{
    using T = typename size_to_uint<SIZE>::type;
    constexpr unsigned BITS = SIZE;

    // RCL count is taken modulo (BITS + 1) for x86-like behavior
    n %= (BITS + 1);
    if (n == 0) {
        return value;
    }

    // Rotate bit-by-bit
    for (unsigned i = 0; i < n; i++) {
        // The top bit (bit BITS-1) becomes the new CF
        bool new_cf = ((value >> (BITS - 1)) & 1) != 0;

        // Shift left by 1
        value <<= 1;

        // Bring old CF into the lowest bit
        if (cf) {
            value |= 1U;  // set bit 0
        } else {
            value &= ~1U; // clear bit 0
        }

        // Mask off any overflow beyond SIZE bits
        // (Typically not needed if T is exactly SIZE bits wide, but it's safe.)
        if constexpr (BITS < (sizeof(T) * 8)) {
            T mask = (T{1} << BITS) - 1;
            value &= mask;
        }

        // Update CF
        cf = new_cf;
    }
    return value;
}

// ---------------------------------------------------------------------------
// 3) RCR: Rotate through carry RIGHT by 'n' bits
//    - value: the SIZE-bit operand
//    - n: number of bits to rotate
//    - cf: the carry flag (single bit, 0 or 1) passed by reference
// ---------------------------------------------------------------------------
template <unsigned SIZE>
  requires (SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64)
constexpr typename size_to_uint<SIZE>::type
rcr(typename size_to_uint<SIZE>::type value, unsigned n, bool &cf)
{
    using T = typename size_to_uint<SIZE>::type;
    constexpr unsigned BITS = SIZE;

    // RCR count is taken modulo (BITS + 1) for x86-like behavior
    n %= (BITS + 1);
    if (n == 0) {
        return value;
    }

    // Rotate bit-by-bit
    for (unsigned i = 0; i < n; i++) {
        // The lowest bit (bit 0) becomes the new CF
        bool new_cf = (value & 1U) != 0;

        // Shift right by 1
        value >>= 1;

        // Bring old CF into the highest bit
        if (cf) {
            value |= (T{1} << (BITS - 1));
        } else {
            // ensure that bit (BITS-1) is cleared
            T mask = static_cast<T>(~(T{1} << (BITS - 1)));
            value &= mask;
        }

        // Mask to SIZE bits if T might be wider
        if constexpr (BITS < (sizeof(T) * 8)) {
            T mask = (T{1} << BITS) - 1;
            value &= mask;
        }

        // Update CF
        cf = new_cf;
    }
    return value;
}
void SysdarftCPUInstructionExecutor::rcl(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();
    bool cf = SysdarftRegister::load<FlagRegisterType>().Carry;
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = ::rcl<8> (static_cast<uint8_t>(operand1), operand2, cf); break;
    case _16bit_prefix: result = ::rcl<16>(static_cast<uint16_t>(operand1), operand2, cf); break;
    case _32bit_prefix: result = ::rcl<32>(static_cast<uint32_t>(operand1), operand2, cf); break;
    case _64bit_prefix: result = ::rcl<64>(operand1, operand2, cf); break;
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
    bool cf = SysdarftRegister::load<FlagRegisterType>().Carry;
    uint64_t result;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  result = ::rcr<8> (static_cast<uint8_t>(operand1), operand2, cf); break;
    case _16bit_prefix: result = ::rcr<16>(static_cast<uint16_t>(operand1), operand2, cf); break;
    case _32bit_prefix: result = ::rcr<32>(static_cast<uint32_t>(operand1), operand2, cf); break;
    case _64bit_prefix: result = ::rcr<64>(operand1, operand2, cf); break;
    default: throw IllegalInstruction("Unknown width");
    }

    auto FG = SysdarftRegister::load<FlagRegisterType>();
    FG.Carry = cf;
    SysdarftRegister::store<FlagRegisterType>(FG);
    WidthAndOperands.second[0].set_val(result);
}
