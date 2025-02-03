/* SysdarftCPUDecoder.cpp
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

#include <type_traits>
#include <iomanip>
#include <InstructionSet.h>
#include <EncodingDecoding.h>
#include <SysdarftCPUDecoder.h>

uint64_t OperandType::do_access_register_based_on_table() const
{
    switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterWidthBCD) {
    case _8bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: return Access.load<RegisterType, 0>();
        case 0x01: return Access.load<RegisterType, 1>();
        case 0x02: return Access.load<RegisterType, 2>();
        case 0x03: return Access.load<RegisterType, 3>();
        case 0x04: return Access.load<RegisterType, 4>();
        case 0x05: return Access.load<RegisterType, 5>();
        case 0x06: return Access.load<RegisterType, 6>();
        case 0x07: return Access.load<RegisterType, 7>();
        default: throw IllegalInstruction("Unknown Register Type");
        }
    case _16bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: return Access.load<ExtendedRegisterType, 0>();
        case 0x01: return Access.load<ExtendedRegisterType, 1>();
        case 0x02: return Access.load<ExtendedRegisterType, 2>();
        case 0x03: return Access.load<ExtendedRegisterType, 3>();
        case 0x04: return Access.load<ExtendedRegisterType, 4>();
        case 0x05: return Access.load<ExtendedRegisterType, 5>();
        case 0x06: return Access.load<ExtendedRegisterType, 6>();
        case 0x07: return Access.load<ExtendedRegisterType, 7>();
        default: throw IllegalInstruction("Unknown Register Type");
        }
    case _32bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: return Access.load<HalfExtendedRegisterType, 0>();
        case 0x01: return Access.load<HalfExtendedRegisterType, 1>();
        case 0x02: return Access.load<HalfExtendedRegisterType, 2>();
        case 0x03: return Access.load<HalfExtendedRegisterType, 3>();
        case 0x04: return Access.load<HalfExtendedRegisterType, 4>();
        case 0x05: return Access.load<HalfExtendedRegisterType, 5>();
        case 0x06: return Access.load<HalfExtendedRegisterType, 6>();
        case 0x07: return Access.load<HalfExtendedRegisterType, 7>();
        default: throw IllegalInstruction("Unknown Register Type");
        }
    case _64bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: return Access.load<FullyExtendedRegisterType, 0>();
        case 0x01: return Access.load<FullyExtendedRegisterType, 1>();
        case 0x02: return Access.load<FullyExtendedRegisterType, 2>();
        case 0x03: return Access.load<FullyExtendedRegisterType, 3>();
        case 0x04: return Access.load<FullyExtendedRegisterType, 4>();
        case 0x05: return Access.load<FullyExtendedRegisterType, 5>();
        case 0x06: return Access.load<FullyExtendedRegisterType, 6>();
        case 0x07: return Access.load<FullyExtendedRegisterType, 7>();
        case 0x08: return Access.load<FullyExtendedRegisterType, 8>();
        case 0x09: return Access.load<FullyExtendedRegisterType, 9>();
        case 0x0a: return Access.load<FullyExtendedRegisterType, 10>();
        case 0x0b: return Access.load<FullyExtendedRegisterType, 11>();
        case 0x0c: return Access.load<FullyExtendedRegisterType, 12>();
        case 0x0d: return Access.load<FullyExtendedRegisterType, 13>();
        case 0x0e: return Access.load<FullyExtendedRegisterType, 14>();
        case 0x0f: return Access.load<FullyExtendedRegisterType, 15>();
        case R_StackBase: return Access.load<StackBaseType>();
        case R_StackPointer: return Access.load<StackPointerType>();
        case R_CodeBase: return Access.load<CodeBaseType>();
        case R_DataBase: return Access.load<DataBaseType>();
        case R_DataPointer: return Access.load<DataPointerType>();;
        case R_ExtendedBase: return Access.load<ExtendedBaseType>();
        case R_ExtendedPointer: return Access.load<ExtendedPointerType>();
        default: throw IllegalInstruction("Unknown Register Type");
        }
    default: throw IllegalInstruction("Unknown Register Width");
    }
}

void OperandType::do_decode_register_without_prefix()
{
    const uint8_t width = Access.pop_code8();
    const auto register_index = Access.pop_code8();
    OperandReferenceTable.OperandType = RegisterOperand;
    OperandReferenceTable.OperandInfo.RegisterValue.RegisterWidthBCD = width;
    OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex = register_index;

#ifdef __DEBUG__
    if (width == _64bit_prefix && register_index > 15)
    {
        switch (register_index) {
        case R_StackBase: OperandReferenceTable.literal = "%SB"; break;
        case R_StackPointer: OperandReferenceTable.literal = "%SP"; break;
        case R_CodeBase: OperandReferenceTable.literal = "%CB"; break;
        case R_DataBase: OperandReferenceTable.literal = "%DB"; break;
        case R_DataPointer: OperandReferenceTable.literal = "%DP"; break;
        case R_ExtendedBase: OperandReferenceTable.literal = "%EB"; break;
        case R_ExtendedPointer: OperandReferenceTable.literal = "%EP"; break;
        default: throw IllegalInstruction("Unknown register index");
        }
    }
    else
    {
        switch (width) {
        case _8bit_prefix:  OperandReferenceTable.literal = "%R"   + std::to_string(register_index); break;
        case _16bit_prefix: OperandReferenceTable.literal = "%EXR" + std::to_string(register_index); break;
        case _32bit_prefix: OperandReferenceTable.literal = "%HER" + std::to_string(register_index); break;
        case _64bit_prefix: OperandReferenceTable.literal = "%FER" + std::to_string(register_index); break;
        default: throw IllegalInstruction("Unknown register type");
        }
    }
#endif // __DEBUG__
}

void OperandType::do_decode_constant_without_prefix()
{
    OperandReferenceTable.OperandType = ConstantOperand;
    const auto & prefix = Access.pop_code8();
    uint64_t num;

#ifdef __DEBUG__
    std::stringstream ss;
    std::string prefix_literal;
#endif // __DEBUG__

    if (prefix == _8bit_prefix) {
#ifdef __DEBUG__
        prefix_literal = "8";
#endif // __DEBUG__
        num = Access.pop_code8();
    } else if (prefix == _16bit_prefix) {
#ifdef __DEBUG__
        prefix_literal = "16";
#endif // __DEBUG__
        num = Access.pop_code16();
    } else if (prefix == _32bit_prefix) {
#ifdef __DEBUG__
        prefix_literal = "32";
#endif // __DEBUG__
        num = Access.pop_code32();
    } else if (prefix == _64bit_prefix) {
#ifdef __DEBUG__
        prefix_literal = "64";
#endif // __DEBUG__
        num = Access.pop_code64();
    } else {
        throw IllegalInstruction("Unknown constant width");
    }

    OperandReferenceTable.OperandInfo.ConstantValue = num;
    OperandReferenceTable.OperandInfo.ConstantWidth = prefix;

#ifdef __DEBUG__
    ss << "0x" << std::uppercase << std::hex << num;
    OperandReferenceTable.literal = "$" + prefix_literal + "(" + ss.str() + ")";
#endif // __DEBUG__
}

template <typename Type, unsigned BitWidth = sizeof(Type) * 8>
bool check_msb(const Type data)
{
    // Ensure the data is treated as unsigned to avoid sign extension issues
    using UnsignedType = std::make_unsigned_t<Type>;
    auto udata = static_cast<UnsignedType>(data);

    // Ensure that the BitWidth is not greater than the size of the type
    static_assert(BitWidth <= sizeof(UnsignedType) * 8, "BitWidth exceeds the size of the type.");

    // Get the most significant bit (MSB) position
    auto msb_off = BitWidth - 1;  // MSB position based on BitWidth
    auto msb = (udata >> msb_off) & 1; // Isolate the MSB

    return msb == 1; // Return true if MSB is 1, false otherwise
}

template < typename Type, unsigned BitWidth = sizeof(Type) * 8 >
int64_t convert_to_64bit_signed(const Type data)
{
    uint64_t compliment = 0;
    uint64_t complimented_data = 0;
    int64_t result = 0;
    switch (BitWidth) {
    case 8: compliment = 0xFFFFFFFFFFFFFF00; break;
    case 16: compliment = 0xFFFFFFFFFFFF0000; break;
    case 32: compliment = 0xFFFFFFFF00000000; break;
    case 64: compliment = 0; break;
    default: throw IllegalInstruction("Unknown bitwidth");
    }

    complimented_data = data | compliment;
    result = *(int64_t*)&complimented_data;
    return result;
}

void OperandType::do_decode_memory_without_prefix()
{
    const auto WidthBCD = Access.pop_code8();
    std::string literal1, literal2, literal3;
    uint64_t base, off1;
    int64_t off2;

    auto decode_each_parameter = [&](std::string & literal, uint64_t & val)
    {
        switch(/*auto prefix = */Access.pop_code8())
        {
        case REGISTER_PREFIX:
            do_decode_register_without_prefix();
            val = do_access_register_based_on_table();
#ifdef __DEBUG__
            literal = OperandReferenceTable.literal;
#endif
            break;
        case CONSTANT_PREFIX:
            do_decode_constant_without_prefix();
#ifdef __DEBUG__
            literal = OperandReferenceTable.literal;
#endif
            val = OperandReferenceTable.OperandInfo.ConstantValue;
            break;
        default: throw IllegalInstruction("Illegal memory operand");
        }
    };

    decode_each_parameter(literal1, base);
    decode_each_parameter(literal2, off1);
    // decode_each_parameter(literal3, off2);

    switch(/*auto prefix = */Access.pop_code8())
    {
    case REGISTER_PREFIX: {
        do_decode_register_without_prefix();
#ifdef __DEBUG__
        literal3 = OperandReferenceTable.literal;
#endif
        const auto val = do_access_register_based_on_table();
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterWidthBCD) {
        case _8bit_prefix:  off2 = check_msb(*(uint8_t*)&val) ?  convert_to_64bit_signed(*(uint8_t*)&val) :  static_cast<int64_t>(val); break;
        case _16bit_prefix: off2 = check_msb(*(uint16_t*)&val) ? convert_to_64bit_signed(*(uint16_t*)&val) : static_cast<int64_t>(val); break;
        case _32bit_prefix: off2 = check_msb(*(uint32_t*)&val) ? convert_to_64bit_signed(*(uint32_t*)&val) : static_cast<int64_t>(val); break;
        case _64bit_prefix: off2 = check_msb(*(uint64_t*)&val) ? convert_to_64bit_signed(*(uint64_t*)&val) : static_cast<int64_t>(val); break;
        default: throw IllegalInstruction("Unknown register width");
        }
        break;
    }
    case CONSTANT_PREFIX: {
        do_decode_constant_without_prefix();
        const auto val = OperandReferenceTable.OperandInfo.ConstantValue;
        switch (OperandReferenceTable.OperandInfo.ConstantWidth) {
        case _8bit_prefix:  off2 = check_msb(*(uint8_t*)&val) ?  convert_to_64bit_signed(*(uint8_t*)&val) :  static_cast<int64_t>(val); break;
        case _16bit_prefix: off2 = check_msb(*(uint16_t*)&val) ? convert_to_64bit_signed(*(uint16_t*)&val) : static_cast<int64_t>(val); break;
        case _32bit_prefix: off2 = check_msb(*(uint32_t*)&val) ? convert_to_64bit_signed(*(uint32_t*)&val) : static_cast<int64_t>(val); break;
        case _64bit_prefix: off2 = check_msb(*(uint64_t*)&val) ? convert_to_64bit_signed(*(uint64_t*)&val) : static_cast<int64_t>(val); break;
        default: throw IllegalInstruction("Unknown register width");
        }
#ifdef __DEBUG__
        literal3 = std::to_string(off2);
#endif
        break;
    }
    default: throw IllegalInstruction("Illegal memory operand");
    }

    const uint8_t ratio_bcd = Access.pop_code8();
    uint8_t ratio = 0;

    switch (ratio_bcd) {
    case 0x01: ratio = 1; break;
    case 0x02: ratio = 2; break;
    case 0x04: ratio = 4; break;
    case 0x08: ratio = 8; break;
    case 0x16: ratio = 16; break;
    default: throw IllegalInstruction("Unknown ratio");
    }

    const uint64_t calculated_address = (base + off1 + off2) * ratio;
    OperandReferenceTable.OperandType = MemoryOperand;
    OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress = calculated_address;
    OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryWidthBCD = WidthBCD;

#ifdef __DEBUG__
    OperandReferenceTable.literal = "*" + std::to_string(ratio) + "&";

    switch (WidthBCD) {
    case _8bit_prefix:  OperandReferenceTable.literal += "8"; break;
    case _16bit_prefix: OperandReferenceTable.literal += "16"; break;
    case _32bit_prefix: OperandReferenceTable.literal += "32"; break;
    case _64bit_prefix: OperandReferenceTable.literal += "64"; break;
    default: throw IllegalInstruction("Unknown width");
    }

    OperandReferenceTable.literal += "(" + literal1 + ", " + literal2 + ", " + literal3 + ")";
#else
    if (WidthBCD != _8bit_prefix
        && WidthBCD != _16bit_prefix
        && WidthBCD != _32bit_prefix
        && WidthBCD != _64bit_prefix)
    {
        throw IllegalInstruction("Unknown width");
    }
#endif // __DEBUG__
}

void OperandType::do_decode_operand()
{
    switch (/*auto prefix = */Access.pop_code8())
    {
        case REGISTER_PREFIX: do_decode_register_without_prefix(); break;
        case CONSTANT_PREFIX: do_decode_constant_without_prefix(); break;
        case MEMORY_PREFIX: do_decode_memory_without_prefix(); break;
        default: throw IllegalInstruction("Unknown operand type");
    }

#ifdef __DEBUG__
    OperandReferenceTable.literal = "<" + OperandReferenceTable.literal + ">";
    if (OperandReferenceTable.OperandType == MemoryOperand) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
           << OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress;
        OperandReferenceTable.literal += " /* = " + ss.str() + " */";
    }
#endif // __DEBUG__
}

uint64_t OperandType::do_access_operand_based_on_table() const
{
    switch (OperandReferenceTable.OperandType) {
    case RegisterOperand: return do_access_register_based_on_table();
    case MemoryOperand:   return do_access_width_specified_access_memory_based_on_table();
    case ConstantOperand: return OperandReferenceTable.OperandInfo.ConstantValue;
    default: throw IllegalInstruction("Unknown error!");
    }
}

void OperandType::store_value_to_operand_based_on_table(const uint64_t value)
{
    switch (OperandReferenceTable.OperandType) {
    case RegisterOperand: store_value_to_register_based_on_table(value); break;
    case MemoryOperand:   store_value_to_memory_based_on_table(value); break;
    default: throw IllegalInstruction("Unknown error!");
    }
}

void OperandType::store_value_to_register_based_on_table(const uint64_t value)
{
    switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterWidthBCD) {
    case _8bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: Access.store<RegisterType, 0>(value); break;
        case 0x01: Access.store<RegisterType, 1>(value); break;
        case 0x02: Access.store<RegisterType, 2>(value); break;
        case 0x03: Access.store<RegisterType, 3>(value); break;
        case 0x04: Access.store<RegisterType, 4>(value); break;
        case 0x05: Access.store<RegisterType, 5>(value); break;
        case 0x06: Access.store<RegisterType, 6>(value); break;
        case 0x07: Access.store<RegisterType, 7>(value); break;
        default: throw IllegalInstruction("Unknown Register Type");
        }
        break;
    case _16bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: Access.store<ExtendedRegisterType, 0>(value); break;
        case 0x01: Access.store<ExtendedRegisterType, 1>(value); break;
        case 0x02: Access.store<ExtendedRegisterType, 2>(value); break;
        case 0x03: Access.store<ExtendedRegisterType, 3>(value); break;
        case 0x04: Access.store<ExtendedRegisterType, 4>(value); break;
        case 0x05: Access.store<ExtendedRegisterType, 5>(value); break;
        case 0x06: Access.store<ExtendedRegisterType, 6>(value); break;
        case 0x07: Access.store<ExtendedRegisterType, 7>(value); break;
        default: throw IllegalInstruction("Unknown Register Type");
        }
        break;
    case _32bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: Access.store<HalfExtendedRegisterType, 0>(value); break;
        case 0x01: Access.store<HalfExtendedRegisterType, 1>(value); break;
        case 0x02: Access.store<HalfExtendedRegisterType, 2>(value); break;
        case 0x03: Access.store<HalfExtendedRegisterType, 3>(value); break;
        case 0x04: Access.store<HalfExtendedRegisterType, 4>(value); break;
        case 0x05: Access.store<HalfExtendedRegisterType, 5>(value); break;
        case 0x06: Access.store<HalfExtendedRegisterType, 6>(value); break;
        case 0x07: Access.store<HalfExtendedRegisterType, 7>(value); break;
        default: throw IllegalInstruction("Unknown Register Type");
        }
        break;
    case _64bit_prefix:
        switch (OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex) {
        case 0x00: Access.store<FullyExtendedRegisterType, 0>(value); break;
        case 0x01: Access.store<FullyExtendedRegisterType, 1>(value); break;
        case 0x02: Access.store<FullyExtendedRegisterType, 2>(value); break;
        case 0x03: Access.store<FullyExtendedRegisterType, 3>(value); break;
        case 0x04: Access.store<FullyExtendedRegisterType, 4>(value); break;
        case 0x05: Access.store<FullyExtendedRegisterType, 5>(value); break;
        case 0x06: Access.store<FullyExtendedRegisterType, 6>(value); break;
        case 0x07: Access.store<FullyExtendedRegisterType, 7>(value); break;
        case 0x08: Access.store<FullyExtendedRegisterType, 8>(value); break;
        case 0x09: Access.store<FullyExtendedRegisterType, 9>(value); break;
        case 0x0a: Access.store<FullyExtendedRegisterType, 10>(value); break;
        case 0x0b: Access.store<FullyExtendedRegisterType, 11>(value); break;
        case 0x0c: Access.store<FullyExtendedRegisterType, 12>(value); break;
        case 0x0d: Access.store<FullyExtendedRegisterType, 13>(value); break;
        case 0x0e: Access.store<FullyExtendedRegisterType, 14>(value); break;
        case 0x0f: Access.store<FullyExtendedRegisterType, 15>(value); break;
        case R_StackBase: Access.store<StackBaseType>(value); break;
        case R_StackPointer: Access.store<StackPointerType>(value); break;
        case R_CodeBase: Access.store<CodeBaseType>(value); break;
        case R_DataBase: Access.store<DataBaseType>(value); break;
        case R_DataPointer: Access.store<DataPointerType>(value); break;
        case R_ExtendedBase: Access.store<ExtendedBaseType>(value); break;
        case R_ExtendedPointer: Access.store<ExtendedPointerType>(value); break;
        default: throw IllegalInstruction("Unknown Register Type");
        }
        break;
    default: throw IllegalInstruction("Unknown Register Width");
    }
}

void OperandType::store_value_to_memory_based_on_table(const uint64_t value)
{
    int width = 0;
    switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryWidthBCD) {
    case _8bit_prefix: width = 1; break;
    case _16bit_prefix: width = 2; break;
    case _32bit_prefix: width = 4; break;;
    case _64bit_prefix: width = 8; break;
    default: throw IllegalInstruction("Unknown Error!");
    }

    // auto DB = Access.load<DataBaseType>();
    Access.write_memory(
        OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress,
        (const char*)&value,
        width);
}

SysdarftCPUInstructionDecoder::ActiveInstructionType
SysdarftCPUInstructionDecoder::pop_instruction_from_ip_and_increase_ip()
{
    uint8_t instruction = 0;
    ActiveInstructionType ret { };
    std::stringstream buffer;

    instruction = pop_code8();

    for (const auto &[fst, snd] : instruction_map)
    {
        if (snd.at(ENTRY_OPCODE) == instruction)
        {
            // register instruction opcode
            ret.opcode = instruction;
            buffer << fst;

            if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
            {
                const auto width = pop_code8();
                ret.width = width;

                switch (width)
                {
                case _8bit_prefix:
#ifdef __DEBUG__
                    buffer << " .8bit ";
#endif
                    break;
                case _16bit_prefix:
#ifdef __DEBUG__
                    buffer << " .16bit";
#endif
                    break;
                case _32bit_prefix:
#ifdef __DEBUG__
                    buffer << " .32bit";
#endif
                    break;
                case _64bit_prefix:
#ifdef __DEBUG__
                    buffer << " .64bit";
#endif
                    break;
                default: throw IllegalInstruction("Unknown width specification");
                }
            }

            const auto arg_count = snd.at(ENTRY_ARGUMENT_COUNT);
            for (uint64_t i = 0 ; i < arg_count; i++)
            {
                ret.operands.emplace_back(*this);
#ifdef __DEBUG__
                buffer << " " << ret.operands.back().get_literal() << (i == 0 && arg_count > 1 ? "," : "");
#endif
            }

#ifdef __DEBUG__
            ret.literal = buffer.str();
#endif
            return ret;
        }
    }

    throw IllegalInstruction("Unknown instruction");
}
