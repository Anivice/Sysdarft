/* EncodeInstruction.cpp
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

#include <vector>
#include <regex>
#include <iomanip>
#include <EncodingDecoding.h>
#include <SysdarftDebug.h>
#include <InstructionSet.h>

const std::regex instruction_pattern(R"(([A-Z-0-9]+))");
const std::regex operation_width(R"(.8BIT|.16BIT|.32BIT|.64BIT)");

std::vector<std::string> clean_line(const std::string & _input)
{
    std::vector<std::string> ret;
    std::string input = _input;

    // capitalize the whole string
    capitalization(input);

    if (std::smatch matches; std::regex_search(input, matches, instruction_pattern)) {
        auto match = matches[0].str();
        ret.emplace_back(remove_space(match));
    } else {
        throw InstructionExpressionError("No match for instruction in " + input);
    }

    if (std::smatch matches; std::regex_search(input, matches, operation_width)) {
        auto match = matches[0].str();
        ret.emplace_back(remove_space(match));
    }

    // Create iterators to traverse all matches
    const auto matches_begin = std::sregex_iterator(input.begin(), input.end(), target_pattern);
    const auto matches_end = std::sregex_iterator();

    // Iterate over all matches and process them
    for (std::sregex_iterator i = matches_begin; i != matches_end; ++i)
    {
        auto match = i->str();
        ret.emplace_back(remove_space(match));
    }

    return ret;
}

uint8_t encode_width_specifier(std::vector<uint8_t> &buffer, const std::string & specifier)
{
    if (specifier == ".8BIT")  { code_buffer_push8(buffer, _8bit_prefix);  return _8bit_prefix;  }
    if (specifier == ".16BIT") { code_buffer_push8(buffer, _16bit_prefix); return _16bit_prefix; }
    if (specifier == ".32BIT") { code_buffer_push8(buffer, _32bit_prefix); return _32bit_prefix; }
    if (specifier == ".64BIT") { code_buffer_push8(buffer, _64bit_prefix); return _64bit_prefix; }

    throw InstructionExpressionError("Unknown specifier " + specifier);
}

void SYSDARFT_EXPORT_SYMBOL encode_instruction(std::vector<uint8_t> & buffer, const std::string & instruction)
{
    const auto cleaned_line = clean_line(instruction);
    if (!instruction_map.contains(cleaned_line[0])) {
        throw InstructionExpressionError("Unknown instruction " + instruction);
    }

    const auto & instruction_name = cleaned_line[0];
    const auto & argument_count = instruction_map.at(cleaned_line[0]).at(ENTRY_ARGUMENT_COUNT);
    const auto & requires_width_specification =
        instruction_map.at(cleaned_line[0]).at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION);

    int operand_index_begin = 1;
    uint8_t current_ops_width = 0;

    code_buffer_push8(buffer, instruction_map.at(instruction_name).at(ENTRY_OPCODE));
    if (requires_width_specification != 0)
    {
        if (cleaned_line.size() < 2) {
            throw InstructionExpressionError("Width specification required but not found for " + instruction);
        }

        try {
            current_ops_width = encode_width_specifier(buffer, cleaned_line[1]);
        } catch (const InstructionExpressionError & /* Err */) {
            throw InstructionExpressionError("Illegal width specification for " + instruction);
        }

        operand_index_begin++;
    }

    std::vector < parsed_target_t > SanityCheckOperandVector;

    for (uint64_t i = 0; i  < argument_count; i++)
    {
        const auto & provided_args = cleaned_line.size() - operand_index_begin;
        if (provided_args < argument_count) {
            throw InstructionExpressionError(
                "Expected " + std::to_string(argument_count) +
                        " operands, but found " + std::to_string(cleaned_line.size() - operand_index_begin) +
                        ": " + instruction + "\n"
                        "NOTE:\n"
                        "    If the missing operand is indeed provided, it's possibly due to a malformed operand expression which, in turn,\n"
                        "    leads to the operand not being detected. Specially, if the operand is a line marker (symbol), then it is\n"
                        "    highly possible that this symbol is not defined, nor being clearly declared, thus the assembler has no\n"
                        "    knowledge of this symbol, and cannot capture it as a valid operand. If one of the operand present above\n"
                        "    is <(0xFFFFFFFFFFFFFFFF)>, and it is a line marker (symbol) in the source code, it is not a malformed operand\n"
                        "    nor an internal error. Assembler has successfully captured it as a valid symbol, and is waiting for linker to\n"
                        "    link the symbol location correctly.");
        }

        if (provided_args > argument_count) {
            throw InstructionExpressionError("Only need " + std::to_string(argument_count)
                + ", but given " + std::to_string(provided_args) + ": " + instruction);
        }

        auto tmp = cleaned_line[i + operand_index_begin];

        // remove < >
        tmp.erase(tmp.begin());
        tmp.pop_back();

        // encode
        parsed_target_t parsed_target;

        try {
            parsed_target = encode_target(buffer, tmp);
            SanityCheckOperandVector.emplace_back(parsed_target);
        } catch (const SysdarftCodeExpressionError & Err) {
            throw InstructionExpressionError("Illegal Target operand expression for " + instruction +
                "\n>>>\n" + Err.what() + "<<<\n");
        }

        auto assertion = [&instruction](const bool & condition) {
            if (!condition) {
                throw InstructionExpressionError("Operation width mismatch for " + instruction);
            }
        };

        // width consistency check
        if (current_ops_width != 0)
        {
            if (parsed_target.TargetType == parsed_target_t::REGISTER)
            {
                switch (current_ops_width)
                {
                    case _8bit_prefix:  assertion(parsed_target.RegisterName[1] == 'R'); break;
                    case _16bit_prefix: assertion(parsed_target.RegisterName[1] == 'E'); break;
                    case _32bit_prefix: assertion(parsed_target.RegisterName[1] == 'H'); break;
                    case _64bit_prefix: assertion(
                        parsed_target.RegisterName[1] == 'F'
                        || parsed_target.RegisterName == "%SB"
                        || parsed_target.RegisterName == "%SP"
                        || parsed_target.RegisterName == "%CB"
                        || parsed_target.RegisterName == "%DB"
                        || parsed_target.RegisterName == "%DP"
                        || parsed_target.RegisterName == "%EB"
                        || parsed_target.RegisterName == "%EP");
                    break;

                    // it should never reach this:
                    default: throw InstructionExpressionError("Unknown error for " + instruction);
                }
            }

            if (parsed_target.TargetType == parsed_target_t::MEMORY)
            {
                switch (current_ops_width)
                {
                case _8bit_prefix:  assertion(parsed_target.memory.MemoryWidth == "8");  break;
                case _16bit_prefix: assertion(parsed_target.memory.MemoryWidth == "16"); break;
                case _32bit_prefix: assertion(parsed_target.memory.MemoryWidth == "32"); break;
                case _64bit_prefix: assertion(parsed_target.memory.MemoryWidth == "64"); break;
                default: throw InstructionExpressionError("Unknown error for " + instruction);
                }
            }

            if (parsed_target.TargetType == parsed_target_t::CONSTANT)
            {
                switch (current_ops_width)
                {
                case _8bit_prefix:  assertion(parsed_target.ConstantWidth == "8");  break;
                case _16bit_prefix: assertion(parsed_target.ConstantWidth == "16"); break;
                case _32bit_prefix: assertion(parsed_target.ConstantWidth == "32"); break;
                case _64bit_prefix: assertion(parsed_target.ConstantWidth == "64"); break;
                default: throw InstructionExpressionError("Unknown error for " + instruction);
                }
            }
        }
    }

    try {
        OperandSanityCheck(instruction_map.at(instruction_name).at(ENTRY_OPCODE), SanityCheckOperandVector);
    } catch (const std::exception & e) {
        throw InstructionExpressionError("Operand sanity check for " + instruction + " failed: " + e.what());
    }
}
