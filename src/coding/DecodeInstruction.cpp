/* DecodeInstruction.cpp
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

#include <iomanip>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

void decode_instruction(std::vector < std::string > & output, std::vector<uint8_t> & input)
{
    try
    {
        std::stringstream buffer;
        const auto instruction = code_buffer_pop8(input);

        for (const auto &[fst, snd] : instruction_map)
        {
            if (snd.at(ENTRY_OPCODE) == instruction)
            {
                buffer << fst;

                uint8_t op_width = 0;
                if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
                {
                    switch (op_width = code_buffer_pop8(input))
                    {
                    case _8bit_prefix:  buffer << " .8bit "; break;
                    case _16bit_prefix: buffer << " .16bit";  break;
                    case _32bit_prefix: buffer << " .32bit";  break;
                    case _64bit_prefix: buffer << " .64bit";  break;
                    default:
                        output.emplace_back(bad_nbit(instruction));
                        output.emplace_back(bad_nbit(op_width));
                        return;
                    }
                }

                for (uint64_t i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
                {
                    std::vector < std::string > operands;
                    try {
                        decode_target(operands, input);
                    } catch (SysdarftBaseError &) {
                        output.emplace_back(bad_nbit(instruction));
                        if (op_width) {
                            output.emplace_back(bad_nbit(op_width));
                        }
                        output.insert(output.end(), operands.begin(), operands.end());
                        return;
                    }

                    buffer << " <";
                    for (const auto & code : operands) {
                        buffer << code;
                    }
                    buffer << ">";

                    if (i == 0 && snd.at(ENTRY_ARGUMENT_COUNT) > 1) {
                        buffer << ",";
                    }
                }

                output.emplace_back(buffer.str());
                return;
            }
        }

        output.emplace_back(bad_nbit(instruction));
    } catch (...) {
        // output.emplace_back("(bad)");
        return;
    }
}
