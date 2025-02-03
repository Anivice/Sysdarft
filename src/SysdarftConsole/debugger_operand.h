/* debugger_operand.h
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

#ifndef DEBUGGER_OPERAND_H
#define DEBUGGER_OPERAND_H

#include <algorithm>
#include <SysdarftCPU.h>

template < unsigned BIT_SIZE >
struct NumTypeIdentifier;

template <>
struct NumTypeIdentifier<8> {
    using type = uint8_t;
};

template <>
struct NumTypeIdentifier<16> {
    using type = uint16_t;
};

template <>
struct NumTypeIdentifier<32> {
    using type = uint16_t;
};

template <>
struct NumTypeIdentifier<64> {
    using type = uint16_t;
};

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



    template <unsigned BIT_SIZE>
        requires(BIT_SIZE % 8 == 0)
    typename NumTypeIdentifier<BIT_SIZE>::type pop_code()
    {
        auto pop8 = [&]() -> uint8_t {
            uint8_t value = 0;
            if (!operand_expression.empty()) {
                value = operand_expression.back();
                operand_expression.pop_back();
            } else {
                throw IllegalInstruction("Operand expression is empty");
            }

            return value;
        };

        uint64_t value = 0;

        if (operand_expression.size() >= BIT_SIZE / 8) {
            for (unsigned int i = 0; i < BIT_SIZE / 8; i++) {
                ((uint8_t *)&value)[i] = pop8();
            }
        } else {
            throw IllegalInstruction("Operand expression is empty");
        }

        return static_cast<typename NumTypeIdentifier<BIT_SIZE>::type>(value);
    }

    uint8_t pop_code8() {
        return pop_code<8>();
    }

    uint16_t pop_code16() {
        return pop_code<16>();
    }

    uint32_t pop_code32() {
        return pop_code<32>();
    }

    uint64_t pop_code64() {
        return pop_code<64>();
    }

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
