/* Assembler.cpp
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

#include <fstream>
#include <vector>
#include <algorithm>
#include <EncodingDecoding.h>

void replace_all(
    std::string & original,
    const std::string & target,
    const std::string & replacement)
{
    if (target.empty()) return; // Avoid infinite loop if target is empty

    size_t pos = 0;
    while ((pos = original.find(target, pos)) != std::string::npos) {
        original.replace(pos, target.length(), replacement);
        pos += replacement.length(); // Move past the replacement to avoid infinite loop
    }
}

const std::regex ascii_value_pattern(R"('([^']*)')");
void process_ascii_value(std::string& input)
{
    std::vector<std::pair<std::string, std::string>> replacements;

    // Create iterators to traverse all matches without modifying input during iteration.
    for (auto it = std::sregex_iterator(input.begin(), input.end(), ascii_value_pattern);
         it != std::sregex_iterator(); ++it)
    {
        const std::smatch& match = *it;

        // Extract the entire quoted substring and the inner content separately.
        std::string full_match = match.str();      // e.g. "'A'"
        std::string inner_content = match.str(1);  // e.g. "A"

        // Validate the inner content.
        if (inner_content.size() != 1) {
            throw std::runtime_error("Error encountered while parsing the ASCII expression: " + full_match);
        }

        // Convert the single character to its decimal ASCII value.
        std::string decimal_value = std::to_string(static_cast<int>(inner_content.at(0)));

        // Save the replacement pair: { substring to replace, replacement string }
        replacements.emplace_back(full_match, decimal_value);
    }

    // Perform all replacements on the original string.
    // This separate loop ensures we modify the input after collecting all matches.
    for (const auto& rep : replacements) {
        replace_all(input, rep.first, rep.second);
    }
}

uint64_t code_size_now(const std::vector < std::vector <uint8_t> > & code)
{
    uint64_t size = 0;
    for (const auto & line : code) {
        size += static_cast<uint64_t>(line.size());
    }

    return size;
}

bool process_resvb(const std::string& input, std::vector <uint8_t> & code)
{
    const std::regex resvb_pattern(R"(\s*\.resvb\s+<(.*)>\s*)", std::regex_constants::icase); // .org 0x123
    if (std::smatch match;
        std::regex_search(input, match, resvb_pattern))
    {
        auto expression = match[1].str();
        process_base16(expression);
        const auto processed_expression = execute_bc(expression);
        const auto count = std::strtoll(processed_expression.c_str(), nullptr, 10);
        for (long long int i = 0; i < count; i++) {
            code.push_back(0x00);
        }
        return true;
    }

    return false;
}

std::string unescapeString(const std::string& input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            // Check the character following the backslash
            char next = input[i + 1];
            switch (next) {
            case 'n': result += '\n'; break;  // Newline
            case 't': result += '\t'; break;  // Tab
            case 'r': result += '\r'; break;  // Carriage return
            case '\\': result += '\\'; break; // Literal backslash
            case '\'': result += '\''; break; // Single quote
            case '\"': result += '\"'; break; // Double quote
                // Add more escape sequences if needed
            default:
                // If not a recognized escape, keep the backslash and the character as-is
                    result += '\\';
                result += next;
                break;
            }
            ++i;  // Skip the next character since it's part of the escape sequence
        } else {
            // Regular character, just append it
            result += input[i];
        }
    }

    return result;
}

bool process_string(std::string& input, std::vector <uint8_t> & code)
{
    const std::regex string_pattern(R"(\s*\.string\s+<\s*\"(.*)\"\s*>\s*)", std::regex_constants::icase);
    if (std::smatch match;
        std::regex_search(input, match, string_pattern))
    {
        // the captured group can be wrong, we have to rely on interation

        // unescape
        input = unescapeString(input);

        // find out fist and last appearance of '"'
        const auto first_of = input.find_first_of('"');
        const auto last_of = input.find_last_of('"');

        // sanity check
        if (first_of == std::string::npos || last_of == std::string::npos || first_of >= last_of) {
            throw SysdarftAssemblerError("Error encountered while parsing string: " + input);
        }

        // process
        const ssize_t length = last_of - first_of - 1;
        if (length < 0) {
            throw SysdarftAssemblerError("Error encountered while parsing string: " + input);
        }
        for (const std::string unescaped_substring = input.substr(first_of + 1, length);
            const auto & c : unescaped_substring)
        {
            code.push_back(c);
        }

        return true;
    }

    return false;
}

bool process_data(const std::string& input, std::vector < data_expression_identifier > & data_processors)
{
    const std::regex data_pattern(R"(\s*\.(8|16|32|64)bit_data\s+<(.*)>\s*)", std::regex_constants::icase);
    if (std::smatch match; std::regex_search(input, match, data_pattern))
    {
        data_expression_identifier identifier { };
        identifier.data_byte_count = std::strtol(match[1].str().c_str(), nullptr, 10) / 8;
        identifier.data_string = match[2].str();
        data_processors.push_back(identifier);
        return true;
    }

    return false;
}

object_t SysdarftAssemble(
    std::vector < std::vector <uint8_t> > & instruction_buffer_set,
    std::vector < std::string > & file,
    uint64_t & origin,
    defined_line_marker_t & appeared_line_markers)
{
    // add an empty entry to eliminate null referencing
    instruction_buffer_set.emplace_back();

    auto is_line_empty = [](std::string line)->bool {
        replace_all(line, " ", "");
        return line.empty();
    };

    auto emplace_marker = [&](const std::string & marker, const uint64_t offset)
    {
        for (auto & defined_marker : appeared_line_markers)
        {
            if (defined_marker.line_marker_name == marker)
            {
                if (defined_marker.is_defined) {
                    throw SysdarftAssemblerError("Multiple definition of " + defined_marker.line_marker_name);
                }

                defined_marker.is_defined = true;
                defined_marker.marker_position = offset;
                return;
            }
        }

        appeared_line_markers.emplace_back( line_marker_t {
            .line_marker_name = marker,
            .marker_position = offset,
            .is_defined = true,
            .referenced = false,
            .loc_it_appeared_in_cur_blk = {}
        } );
    };

    auto add_marker_reference = [&](const std::string & marker, const uint64_t offset)
    {
        for (auto & defined_marker : appeared_line_markers)
        {
            if (defined_marker.line_marker_name == marker) {
                defined_marker.loc_it_appeared_in_cur_blk.emplace_back(offset);
                defined_marker.referenced = true;
                return;
            }
        }

        appeared_line_markers.emplace_back(line_marker_t {
            .line_marker_name = marker,
            .marker_position = 0,
            .is_defined = false,
            .referenced = true,
            .loc_it_appeared_in_cur_blk = { offset } });
    };

    std::vector < data_expression_identifier > data_processors;
    uint64_t line_number = 0;

    for (auto & line : file)
    {
        line_number++;

        if (is_line_empty(line)) {
            continue;
        }

        if (std::smatch match; std::regex_match(line, match, line_mark_pattern))
        {
            // found a line marker in this line
            auto marker = match[1].str();

            // register current offset
            emplace_marker(marker, code_size_now(instruction_buffer_set) + origin);
            continue;
        }

        try {
            std::vector <uint8_t> code_for_this_instruction;

            // preprocessor .string expression
            if (process_string(line, code_for_this_instruction)) {
                instruction_buffer_set.emplace_back(code_for_this_instruction);
                continue; // preprocessor that does not need to be compiled
            }

            // preprocessor @@ (org)
            if (line.find("@@") != std::string::npos) {
                replace_all(line, "@@", std::to_string(origin));
            }

            // preprocessor @ (current offset)
            if (line.find('@') != std::string::npos) {
                replace_all(line,
                    "@",
                    std::to_string(code_size_now(instruction_buffer_set) + origin));
            }

            // preprocessor .resvb expression
            if (process_resvb(line, code_for_this_instruction)) {
                instruction_buffer_set.emplace_back(code_for_this_instruction);
                continue; // preprocessor that does not need to be compiled
            }

            // preprocessor 'ASCII'
            process_ascii_value(line);

            // process data
            if (process_data(line, data_processors))
            {
                data_processors.back().data_appearance = instruction_buffer_set.size() - 1; // mark its location
                instruction_buffer_set.emplace_back(data_processors.back().data_byte_count);
                continue; // this preprocessor is the most complicated.
                // it needs to handle @ and @@ and all line markers, turn them into actual offsets,
                // then calculate the processed expression using bc
            }

            // match appearances of marker operand
            for (const auto & line_marker : appeared_line_markers)
            {
                std::smatch matches;
                if (std::regex marker(R"((.*)(<\s*)" + line_marker.line_marker_name + R"(\s*>)(.*))");
                    std::regex_search(line, matches, marker))
                {
                    if (matches.size() != 4) {
                        throw SysdarftAssemblerError("Error encountered while parsing line marker: " + line);
                    }

                    replace_all(line, matches[2], "<$(0xFFFFFFFFFFFFFFFF)>");
                    add_marker_reference(line_marker.line_marker_name, instruction_buffer_set.size() - 1);
                    break;
                }
            }

            encode_instruction(code_for_this_instruction, line);
            instruction_buffer_set.emplace_back(code_for_this_instruction);
        } catch (std::exception & e) {
            throw SysdarftAssemblerError(std::string("Line: ") + std::to_string(line_number)
                + ": Error occurred when compiling: " + std::string(e.what()));
        }
    }

    // pop empty entry
    instruction_buffer_set.erase(instruction_buffer_set.begin());

    // add offset to origin
    origin += code_size_now(instruction_buffer_set);

    auto ret = object_t {
        .code = instruction_buffer_set,
        .symbol_table = appeared_line_markers,
        .data_expression_identifiers = data_processors };

    // strip line marker processor identifications
    for (auto & marker : appeared_line_markers) {
        marker.loc_it_appeared_in_cur_blk.clear();
    }

    return ret;
}
