#ifndef DEBUGGER_OPERAND_H
#define DEBUGGER_OPERAND_H

#include <algorithm>
#include <SysdarftCPU.h>

// short-lived type, valid only for current timestamp
class debugger_operand_type
{
protected:
    DecoderDataAccess & Access;
    std::vector < uint8_t > operand_expression;

    enum OperandType_t { NaO, RegisterOperand, ConstantOperand, MemoryOperand };

    struct
    {
        OperandType_t OperandType;

        struct {
            uint64_t ConstantValue;
            struct {
                uint8_t RegisterWidthBCD;
                uint8_t RegisterIndex;
            } RegisterValue;

            struct {
                uint64_t MemoryAddress;
                uint8_t RegisterWidthBCD;
            } CalculatedMemoryAddress;
        } OperandInfo { };

        std::string literal; // literal will be wrong for all FPU and signed instructions
                             // since it decodes to unsigned only.
                             // These instructions have to output correct literals manually
    } OperandReferenceTable { };

    uint64_t do_access_register_based_on_table();

    template < typename DataType >
    DataType do_width_ambiguous_access_memory_based_on_table()
    {
        DataType ret { };
        const auto DB = Access.load<DataBaseType>();
        const auto DP = OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress;
        Access.read_memory(DB + DP, (char*)&ret, sizeof(DataType));
        return ret;
    }

    uint64_t do_access_width_specified_access_memory_based_on_table();

    void store_value_to_register_based_on_table(uint64_t value);
    void store_value_to_memory_based_on_table(uint64_t value);

    void do_decode_register_without_prefix();
    void do_decode_constant_without_prefix();
    void do_decode_memory_without_prefix();
    void do_decode_operand();

    uint64_t do_access_operand_based_on_table();
    void store_value_to_operand_based_on_table(uint64_t value);

    uint8_t pop_code8();
    uint64_t pop_code64();

public:
    [[nodiscard]] uint64_t get_val() { return do_access_operand_based_on_table(); }
    [[nodiscard]] uint64_t get_effective_addr() const { return OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress; }
    void set_val(const uint64_t val) { store_value_to_operand_based_on_table(val); }
    [[nodiscard]] std::string get_literal() const { return OperandReferenceTable.literal; }
    explicit debugger_operand_type(DecoderDataAccess & Access_,
        const std::vector<uint8_t> &operand_expression_)
    : Access(Access_), operand_expression(operand_expression_)
    {
        std::ranges::reverse(operand_expression);
        do_decode_operand();
    }
};

#endif //DEBUGGER_OPERAND_H
