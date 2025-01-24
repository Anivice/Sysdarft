/* DecodeTarget.cpp
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

void decode_constant(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::stringstream ret;
    const auto & prefix = code_buffer_pop<uint8_t>(input);

    if (prefix == _64bit_prefix) {
        const auto num = code_buffer_pop<uint64_t>(input);
        ret << "$(0x" << std::hex << std::uppercase << num << ")";
    } else {
        ret << bad_nbit(prefix);
    }

    output.emplace_back(ret.str());
}

void decode_register(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::stringstream ret;
    const auto register_size = code_buffer_pop<uint8_t>(input);
    const auto register_index = code_buffer_pop<uint8_t>(input);

    std::string prefix = "%";

    switch (register_size)
    {
    case _8bit_prefix:  prefix += "R"; break;
    case _16bit_prefix: prefix += "EXR"; break;
    case _32bit_prefix: prefix += "HER"; break;
    case _64bit_prefix:
        if (register_index <= 15) {
            prefix += "FER";
            break;
        }

        switch (register_index)
        {
        case R_StackBase:       output.emplace_back("%SB"); return;
        case R_StackPointer:    output.emplace_back("%SP"); return;
        case R_CodeBase:        output.emplace_back("%CB"); return;
        case R_DataBase:        output.emplace_back("%DB"); return;
        case R_DataPointer:     output.emplace_back("%DP"); return;
        case R_ExtendedBase:    output.emplace_back("%EB"); return;
        case R_ExtendedPointer: output.emplace_back("%EP"); return;
        default:
            output.emplace_back(bad_nbit(register_size));
            output.emplace_back(bad_nbit(register_index));
            return;
        }

    default:
        output.emplace_back(bad_nbit(register_size));
        output.emplace_back(bad_nbit(register_index));
        return;
    }

    ret << prefix << static_cast<int>(register_index);

    output.push_back(ret.str());
}

void decode_memory(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::string width, ratio;
    std::vector<std::string> operands;
    std::stringstream ret;

    switch (const auto code = code_buffer_pop<uint8_t>(input)) {
    case 0x08: width = "&8"; break;
    case 0x16: width = "&16"; break;
    case 0x32: width = "&32"; break;
    case 0x64: width = "&64"; break;
    default: output.emplace_back(bad_nbit(code)); return;
    }

    auto decode_each_parameter = [&operands, &input]()->bool
    {
        switch(const auto code = code_buffer_pop<uint8_t>(input))
        {
        case REGISTER_PREFIX: decode_register(operands, input); break;
        case CONSTANT_PREFIX: decode_constant(operands, input); break;
        default: operands.emplace_back(bad_nbit(code)); return false;
        }

        return true;
    };

    operands.emplace_back("(");
    if (!decode_each_parameter()) { return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { return; }
    operands.emplace_back(")");

    switch (const auto code = code_buffer_pop<uint8_t>(input))
    {
    case 0x01: ratio = "*1";  break;
    case 0x02: ratio = "*2";  break;
    case 0x04: ratio = "*4";  break;
    case 0x08: ratio = "*8";  break;
    case 0x16: ratio = "*16"; break;
    default: output.emplace_back(bad_nbit(code)); return;
    }

    ret << ratio <<  width;
    for (const auto & operand : operands) {
        ret << operand;
    }

    output.push_back(ret.str());
}

void decode_target(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    switch (const auto code = code_buffer_pop<uint8_t>(input))
    {
    case REGISTER_PREFIX: decode_register(output, input); break;
    case CONSTANT_PREFIX: decode_constant(output, input); break;
    case MEMORY_PREFIX: decode_memory(output, input); break;
    default: output.emplace_back(bad_nbit(code));
    }
}
