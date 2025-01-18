#include "debugger_operand.h"
#include <cstring>
#include <iomanip>
#include <InstructionSet.h>
#include <EncodingDecoding.h>
#include <SysdarftCPUDecoder.h>

uint64_t debugger_operand_type::do_access_register_based_on_table()
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

void debugger_operand_type::do_decode_register_without_prefix()
{
    const uint8_t width = pop_code8();
    const auto register_index = pop_code8();
    OperandReferenceTable.OperandType = RegisterOperand;
    OperandReferenceTable.OperandInfo.RegisterValue.RegisterWidthBCD = width;
    OperandReferenceTable.OperandInfo.RegisterValue.RegisterIndex = register_index;

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
}

void debugger_operand_type::do_decode_constant_without_prefix()
{
    if (const auto & prefix = pop_code8();
        prefix == _64bit_prefix)
    {
        std::stringstream ss;
        const auto num = pop_code64();
        OperandReferenceTable.OperandType = ConstantOperand;
        OperandReferenceTable.OperandInfo.ConstantValue = num;
        ss << "0x" << std::uppercase << std::hex << num;
        OperandReferenceTable.literal = "$(" + ss.str() + ")";
    } else {
        throw IllegalInstruction("Unknown constant width");
    }
}

void debugger_operand_type::do_decode_memory_without_prefix()
{
    const auto WidthBCD = pop_code8();
    std::string literal1, literal2, literal3;
    uint64_t base, off1, off2;

    auto decode_each_parameter = [&](std::string & literal, uint64_t & val)
    {
        switch(/*auto prefix = */pop_code8())
        {
        case REGISTER_PREFIX:
            do_decode_register_without_prefix();
            val = do_access_register_based_on_table();
            literal = OperandReferenceTable.literal;
            break;
        case CONSTANT_PREFIX:
            do_decode_constant_without_prefix();
            literal = OperandReferenceTable.literal;
            val = OperandReferenceTable.OperandInfo.ConstantValue;
            break;
        default: throw IllegalInstruction("Illegal memory operand");
        }
    };

    decode_each_parameter(literal1, base);
    decode_each_parameter(literal2, off1);
    decode_each_parameter(literal3, off2);
    const uint8_t ratio = pop_code8();

    const uint64_t calculated_address = (base + off1 + off2) * ratio;
    OperandReferenceTable.OperandType = MemoryOperand;
    OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress = calculated_address;
    OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.RegisterWidthBCD = WidthBCD;
    OperandReferenceTable.literal = "*" + std::to_string(ratio) + "&";

    switch (WidthBCD) {
    case _8bit_prefix:  OperandReferenceTable.literal += "8"; break;
    case _16bit_prefix: OperandReferenceTable.literal += "16"; break;
    case _32bit_prefix: OperandReferenceTable.literal += "32"; break;
    case _64bit_prefix: OperandReferenceTable.literal += "64"; break;
    default: throw IllegalInstruction("Unknown width");
    }

    OperandReferenceTable.literal += "(" + literal1 + ", " + literal2 + ", " + literal3 + ")";
}

void debugger_operand_type::do_decode_operand()
{
    switch (/*auto prefix = */pop_code8())
    {
        case REGISTER_PREFIX: do_decode_register_without_prefix(); break;
        case CONSTANT_PREFIX: do_decode_constant_without_prefix(); break;
        case MEMORY_PREFIX: do_decode_memory_without_prefix(); break;
        default: throw IllegalInstruction("Unknown operand type");
    }

    OperandReferenceTable.literal = "<" + OperandReferenceTable.literal + ">";
    if (OperandReferenceTable.OperandType == MemoryOperand) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
           << OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress;
        OperandReferenceTable.literal += " /* = " + ss.str() + " */";
    }
}

uint64_t debugger_operand_type::do_access_operand_based_on_table()
{
    switch (OperandReferenceTable.OperandType) {
    case RegisterOperand: return do_access_register_based_on_table();
    case MemoryOperand:   return do_access_width_specified_access_memory_based_on_table();
    case ConstantOperand: return OperandReferenceTable.OperandInfo.ConstantValue;
    default: throw IllegalInstruction("Unknown error!");
    }
}

void debugger_operand_type::store_value_to_operand_based_on_table(const uint64_t value)
{
    switch (OperandReferenceTable.OperandType) {
    case RegisterOperand: store_value_to_register_based_on_table(value); break;
    case MemoryOperand:   store_value_to_memory_based_on_table(value); break;
    default: throw IllegalInstruction("Unknown error!");
    }
}

void debugger_operand_type::store_value_to_register_based_on_table(const uint64_t value)
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

void debugger_operand_type::store_value_to_memory_based_on_table(const uint64_t value)
{
    int width = 0;
    switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.RegisterWidthBCD) {
    case _8bit_prefix:
        width = 1;
        break;
    case _16bit_prefix:
        width = 2;
        break;
    case _32bit_prefix:
        width = 4;
        break;
        ;
    case _64bit_prefix:
        width = 8;
        break;
    default:
        throw IllegalInstruction("Unknown Error!");
    }

    const auto DB = Access.load<DataBaseType>();
    Access.write_memory(
        DB + OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress,
        (const char*)&value,
        width);
}

uint64_t debugger_operand_type::do_access_width_specified_access_memory_based_on_table()
{
    switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.RegisterWidthBCD) {
    case _8bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint8_t>();
    case _16bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint16_t>();
    case _32bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint32_t>();
    case _64bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint64_t>();
    default: throw IllegalInstruction("Unknown memory access width");
    }
}

uint8_t debugger_operand_type::pop_code8()
{
    uint8_t value = 0;
    if (!operand_expression.empty()) {
        value = operand_expression.back();
        operand_expression.pop_back();
    } else {
        throw IllegalInstruction("Operand expression is empty");
    }

    return value;
}

uint64_t debugger_operand_type::pop_code64()
{
    uint64_t value = 0;

    if (operand_expression.size() >= 8)
    {
        for (int i = 0; i < 8; i++) {
            ((char*)&value)[i] = pop_code8();
        }
    } else {
        throw IllegalInstruction("Operand expression is empty");
    }

    return value;
}
