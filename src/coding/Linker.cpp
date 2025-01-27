/* Linker.cpp
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

#include <EncodingDecoding.h>

void encode_constant_from_uint64_t(std::vector < uint8_t > & code, uint64_t val)
{
    // always 10 bytes
    code.push_back(CONSTANT_PREFIX);
    code.push_back(_64bit_prefix);
    for (uint64_t i = 0; i < sizeof(uint64_t); i++) {
        code.push_back(((uint8_t*)&val)[i]);
    }
}

const std::vector < uint8_t > tmp_address_hex = {
    0x02, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
}; // 33 bytes

size_t countSequence(const std::vector<uint8_t>& data, const std::vector<uint8_t>& sequence)
{
    if (sequence.empty() || data.size() < sequence.size()) {
        return 0;
    }

    size_t count = 0;
    auto it = data.begin();

    while (it != data.end())
    {
        it = std::search(it, data.end(), sequence.begin(), sequence.end());
        if (it != data.end()) {
            ++count;
            it += static_cast<long>(sequence.size()); // Non-overlapping
        }
    }

    return count;
}

void replaceSequence(
    std::vector<uint8_t>& original,
    const std::vector<uint8_t>& target,
    const std::vector<uint8_t>& replacement)
{
    if (target.empty()) {
        return;
    }

    // Use std::search to find the first occurrence of target in original

    if (auto it = std::ranges::search(original, target).begin();
        it != original.end())
    {
        // Erase the target sequence
        it = original.erase(it, it + static_cast<long>(target.size()));

        // Insert the replacement sequence
        original.insert(it, replacement.begin(), replacement.end());
    }
}

object_t SysdarftLink(std::vector < object_t > & objects)
{
    object_t linked;
    struct symbol_t {
        uint64_t address;
        bool referenced;
        bool defined;
    };

    std::map < std::string /* marker */, symbol_t /* offset */ > unified_symbol_table;

    // 1. unified symbol table, add all defined symbols
    // 1.1 register all symbols
    for (const auto & object : objects)
    {
        for (const auto & symbol : object.symbol_table)
        {
            if (symbol.is_defined)
            {
                if (unified_symbol_table.contains(symbol.line_marker_name)
                    && unified_symbol_table.at(symbol.line_marker_name).defined) {
                    throw SysdarftLinkerError("Multiple definitions of " + symbol.line_marker_name);
                }

                auto ref = unified_symbol_table[symbol.line_marker_name];
                ref.address = symbol.marker_position;
                ref.defined = true;
                unified_symbol_table[symbol.line_marker_name] = ref;
            }

            if (symbol.referenced)
            {
                auto ref = unified_symbol_table[symbol.line_marker_name];
                ref.referenced = true;
                unified_symbol_table[symbol.line_marker_name] = ref;
            }
        }
    }

    // 2. link all symbols
    for (auto & object : objects)
    {
        // replace line markers with actual appearances
        for (const auto & symbol : object.symbol_table)
        {
            std::vector < uint8_t > replacement;
            try {
                const auto symbol_ref_entry = unified_symbol_table.at(symbol.line_marker_name);
                if (!symbol_ref_entry.defined) {
                    throw SysdarftLinkerError("Undefined reference to " + symbol.line_marker_name);
                }
                encode_constant_from_uint64_t(replacement, symbol_ref_entry.address);
            } catch (const std::out_of_range &) {
                throw SysdarftLinkerError("Undefined reference to " + symbol.line_marker_name);
            }
            for (const auto & each_instruction : symbol.loc_it_appeared_in_cur_blk) {
                if (countSequence(object.code[each_instruction], tmp_address_hex) != 1) {
                    throw SysdarftLinkerError("Overlapping or non-exist code references of the "
                        + symbol.line_marker_name + ",\n"
                        "possibly due to multiple noise data 0xFFFFFFFFFFFFFFFF within code disrupting normal sequence identification.");
                }
                replaceSequence(object.code[each_instruction], tmp_address_hex, replacement);
            }
        }

        // process data
        for (const auto & [ data_appearance, data_byte_count, data_string ]: object.data_expression_identifiers)
        {
            auto expression = data_string;
            // handle all line markers, turn them into actual offsets,
            for (const auto & marker : object.symbol_table) {
                try {
                    replace_all(expression, marker.line_marker_name,
                        std::to_string(unified_symbol_table.at(marker.line_marker_name).address));
                } catch (const std::out_of_range &) {
                    throw SysdarftLinkerError("Undefined reference to " + marker.line_marker_name);
                }
            }

            // then calculate the processed expression using bc
            process_base16(expression);
            auto processed_expression = execute_bc(expression);

            // actual number, signed
            auto data = strtoll(processed_expression.c_str(), nullptr, 10);
            uint64_t compliment = 0xFFFFFFFFFFFFFFFF;
            compliment = compliment >> (64 - (data_byte_count * 8));
            uint64_t raw_data = *(uint64_t*)&data;
            raw_data = raw_data & compliment;

            // emplace data
            std::vector < uint8_t > data_sequence;
            for (uint64_t i = 0; i < data_byte_count; i++) {
                data_sequence.emplace_back(((uint8_t*)&raw_data)[i]);
            }

            object.code[data_appearance] = data_sequence;
        }
    }

    // 3. produce output
    // 3.1 add all code
    for (const auto & object : objects) {
        linked.code.insert(linked.code.end(), object.code.begin(), object.code.end());
    }

    std::vector < uint64_t > loc_it_appeared_in_cur_blk;
    // 3.2 add unified symbol table
    for (const auto & [symbol, value] : unified_symbol_table)
    {
        if (debug::verbose)
        {
            linked.symbol_table.emplace_back(line_marker_t {
                .line_marker_name = symbol,
                .marker_position = value.address,
                .is_defined = false,
                .referenced = false,
                .loc_it_appeared_in_cur_blk = {}
            });
        }
        else
        {
            if (value.referenced)
            {
                linked.symbol_table.emplace_back(line_marker_t {
                    .line_marker_name = symbol,
                    .marker_position = value.address,
                    .is_defined = false,
                    .referenced = false,
                    .loc_it_appeared_in_cur_blk = {}
                });
            }
        }
    }

    return linked;
}
