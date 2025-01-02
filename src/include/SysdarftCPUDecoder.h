#ifndef SYSDARFTCPUINSTRUCTIONDECODER_H
#define SYSDARFTCPUINSTRUCTIONDECODER_H

#include <SysdarftDebug.h>
#include <SysdarftRegister.h>
#include <SysdarftMemory.h>
#include <EncodingDecoding.h>

class IllegalInstruction final : public SysdarftBaseError {
public:
    explicit IllegalInstruction(const std::string & msg) :
        SysdarftBaseError("Illegal instruction: " + msg) { }
};

class OperandType;

class DecoderDataAccess : public SysdarftRegister, public SysdarftCPUMemoryAccess {
protected:
    template < typename DataType >
    DataType pop_code_and_inc_ip()
    {
        DataType result { };
        const auto CB = SysdarftRegister::load<CodeBaseType>();
        auto IP = SysdarftRegister::load<InstructionPointerType>();
        result = pop_memory_from<DataType>(CB, IP);
        SysdarftRegister::store<InstructionPointerType>(IP);
        return result;
    }

    uint8_t pop_code8() {
        return pop_code_and_inc_ip<uint8_t>();
    }

    uint16_t pop_code16() {
        return pop_code_and_inc_ip<uint16_t>();
    }

    uint32_t pop_code32() {
        return pop_code_and_inc_ip<uint32_t>();
    }

    uint64_t pop_code64() {
        return pop_code_and_inc_ip<uint64_t>();
    }

public:
    friend class OperandType;
};

// short-lived type, valid only for current timestamp
class SYSDARFT_EXPORT_SYMBOL OperandType
{
protected:
    DecoderDataAccess & Access;

    enum OperandType_t { NaO, RegisterOperand, ConstantOperand, MemoryOperand };

    struct
    {
        OperandType_t OperandType;

        union
        {
            uint64_t ConstantValue;

            struct {
                uint8_t RegisterWidthBCD;
                uint8_t RegisterIndex;
            } RegisterValue;

            struct {
                uint64_t MemoryAddress;
                uint8_t RegisterWidthBCD;
            } CalculatedMemoryAddress;
        } OperandInfo;

        std::string literal; // literal will be wrong for all FPU and signed instructions
                             // since it decodes to unsigned only.
                             // These instructions have to output correct literals manually
    } OperandReferenceTable { };

    uint64_t do_access_register_based_on_table();

    template < typename DataType >
    DataType do_width_ambiguous_access_memory_based_on_table()
    {
        return Access.pop_memory_from<DataType>(
            Access.load<DataBaseType>(),
            OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress);
    }

    uint64_t do_access_width_specified_access_memory_based_on_table()
    {
        switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.RegisterWidthBCD) {
        case _8bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint8_t>();
        case _16bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint16_t>();
        case _32bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint32_t>();
        case _64bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint64_t>();
        default: throw IllegalInstruction("Unknown memory access width");
        }
    }

    void store_value_to_register_based_on_table(uint64_t value);
    void store_value_to_memory_based_on_table(uint64_t value);

    void do_decode_register_without_prefix();
    void do_decode_constant_without_prefix();
    void do_decode_memory_without_prefix();
    void do_decode_operand();

    uint64_t do_access_operand_based_on_table();
    void store_value_to_operand_based_on_table(uint64_t value);

public:
    [[nodiscard]] uint64_t get_val() { return do_access_operand_based_on_table(); }
    void set_val(const uint64_t val) { store_value_to_operand_based_on_table(val); }
    [[nodiscard]] std::string get_literal() const { return OperandReferenceTable.literal; }
    explicit OperandType(DecoderDataAccess & Access_) : Access(Access_) { do_decode_operand(); }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInstructionDecoder : public DecoderDataAccess
{
protected:
    struct ActiveInstructionType {
        uint8_t opcode;
        uint8_t width;
        std::vector< OperandType > operands;
        std::string literal; // again, literals for FPU and signed operations are all wrong
                             // it will remain this way to reduce the complicity
                             // manually output these instruction literals
                             // instead of relying on automatic literalization
    };

    ActiveInstructionType pop_instruction_from_ip_and_increase_ip();
};

#endif //SYSDARFTCPUINSTRUCTIONDECODER_H
