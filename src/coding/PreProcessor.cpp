/* PreProcessor.cpp
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

#include <regex>
#include <string>
#include <iomanip>
#include <ranges>
#include <EncodingDecoding.h>

const std::regex org_pattern(R"(\s*\.org\s+((?:0x[0-9A-Fa-f]+)|(?:\d+))\s*)", std::regex_constants::icase); // .org 0x123
const std::regex lab_pattern(R"(\s*\.lab\s+([A-Za-z._][A-Za-z0-9._]*(?:\s*,\s*[A-Za-z._][A-Za-z0-9._]*)*)\s*)",
    std::regex_constants::icase);
const std::regex equ_pattern(R"(\s*\.equ\s+'([^']*)'\s*,\s*'([^']*)'\s*)", std::regex_constants::icase);

std::vector<std::string> splitString(const std::string& input, const char delimiter = ',')
{
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

std::string truncateAfterSemicolonOrHash(const std::string& input)
{
    // Find the first occurrence of either ';' or '#'
    // If found, return substring from beginning to that position
    if (const size_t pos = input.find_first_of(";#");
        pos != std::string::npos)
    {
        return input.substr(0, pos);
    }
    // If neither found, return the original string
    return input;
}

// Function to replace whole words based on custom allowed characters
void replace_whole_word(std::string& text,
                                std::string target,
                                const std::string& replacement)
{
    replace_all(target, ".", "\\.");
    const std::string target_reg_literal = R"((^|[^A-Za-z0-9._]))" + target + R"(([^A-Za-z0-9._]|$))";
    const std::regex rep_tag(target_reg_literal);
    std::smatch match;
    std::regex_search(text, match, rep_tag);
    if (!match.empty())
    {
        auto matched = match[0].str();
        matched.pop_back();
        matched.erase(matched.begin());
        replace_all(text, matched, replacement);
    }
}

std::vector < std::string >
line_marker_register(std::vector<std::string> & file)
{
    std::string upper_line_marker;
    std::vector < std::string > result;
    std::map < std::string, std::string > sub_linemarkers;
    std::vector < std::string > current_file_section;
    std::vector < std::vector < std::string > > processed_file_sections;

    for (auto & line : file)
    {
        if (std::regex_match(line, line_mark_pattern))
        {
            // discard spaces and tab
            replace_all(line, " ", "");
            replace_all(line, "\t", "");
            replace_all(line, ":", "");

            const std::string before = line;

            // being a submarker
            if (line.find(".") != std::string::npos)
            {
                // attach line marker
                replace_all(line, ".", "_");

                // process
                line = upper_line_marker + line;

                if (std::ranges::contains(result, line)) {
                    throw SysdarftPreProcessorError("Multiple definition of line markers for " + line);
                }

                sub_linemarkers.emplace(before, line);
                result.emplace_back(line);
                line += ":";
                current_file_section.emplace_back(line);
            }
            else
            {
                current_file_section.emplace_back(line + ":");

                for (std::string & cursf_line : current_file_section)
                {
                    for (const auto & [marker, rep] : sub_linemarkers) {
                        replace_whole_word(cursf_line, marker, rep);
                    }
                }

                processed_file_sections.emplace_back(current_file_section);
                current_file_section.clear();
                sub_linemarkers.clear();

                upper_line_marker = line;

                if (std::ranges::contains(result, line)) {
                    throw SysdarftPreProcessorError("Multiple definition of line markers for " + line);
                }

                result.emplace_back(line);
                line += ":";
            }
        }
        else
        {
            current_file_section.emplace_back(line);
        }
    }

    if (!current_file_section.empty()) {
        processed_file_sections.emplace_back(current_file_section);
    }

    std::vector < std::string > flat_file;

    for (auto & section : processed_file_sections)
    {
        for (const auto & line : section) {
            flat_file.emplace_back(line);
        }

    }

    file = flat_file;

    return result;
}

std::string convertEscapeSequences(const std::string& input)
{
    std::string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            // Check the character following the backslash
            switch (const char nextChar = input[i + 1]) {
            case 'n':
                output.push_back('\n');
                break;
            case 't':
                output.push_back('\t');
                break;
            case '\\':
                output.push_back('\\');
                break;
            case '"':
                output.push_back('\"');
                break;
                // Add additional escape sequence cases as needed
            default:
                // If unknown escape, keep the backslash and next char as is
                    output.push_back('\\');
                output.push_back(nextChar);
                break;
            }
            i++;  // Skip the next character because we've processed it
        } else {
            output.push_back(input[i]);
        }
    }

    return output;
}

void process_org(const std::string& input, uint64_t & org)
{
    // .org [NUM]
    if (std::smatch matches; std::regex_search(input, matches, org_pattern))
    {
        auto num_literal = matches[1].str();
        process_base16(num_literal);
        org = std::strtoll(num_literal.c_str(), nullptr, 10);
    }
}

void process_lab(const std::string& input, defined_line_marker_t & defined_line_marker)
{
    auto is_line_marker_present = [&](const std::string & marker)->bool
    {
        for (const auto & line_marker : defined_line_marker) {
            if (line_marker.line_marker_name == marker) {
                return true;
            }
        }

        return false;
    };

    // .lab marker1, [marker2, ...]
    if (std::smatch matches; std::regex_search(input, matches, lab_pattern))
    {
        auto marker_literal = matches[1].str();
        replace_all(marker_literal, " ", "");
        for (const auto line_markers = splitString(marker_literal);
            const auto & line_marker : line_markers)
        {
            if (!is_line_marker_present(line_marker)) {
                defined_line_marker.emplace_back(line_marker_t {
                    .line_marker_name = line_marker,
                    .marker_position = 0,
                    .is_defined = false,
                    .referenced = false,
                    .loc_it_appeared_in_cur_blk = {}
                });
            }
        }
    }
}

void process_equ(const std::string& input, std::map < std::string, std::string > & equ_replacement)
{
    // .equ 'Extended Regular Expression', 'Replacement'
    // this is marked, process is done when the whole block is processed before compile
    if (std::smatch matches; std::regex_search(input, matches, equ_pattern)) {
        auto operand1 = matches[1].str();
        auto operand2 = matches[2].str();
        equ_replacement.emplace(operand1, operand2);
    }
}

void sed_equ(std::string& input, std::map < std::string, std::string > & equ_replacement, const bool regex)
{
    for (const auto & [key, value] : equ_replacement)
    {
        if (regex)
        {
            const auto grep = debug::exec_command("/usr/bin/grep", input, "-E", key);
            if (grep.exit_status == 0)
            {
                std::stringstream ss;
                ss << "s/" << key << "/" << value << "/g";
                const auto result = debug::exec_command("/usr/bin/sed", input, "-E", ss.str());
                if (result.exit_status != 0) {
                    throw SysdarftPreProcessorError("Failed to process .equ using sed: " + result.fd_stderr);
                }

                input = result.fd_stdout;
            }

            // we have to warn the user if grep simply cannot be found, NOT because there is NOT a match
            if (grep.exit_status == 127) {
                // command not found
                throw SysdarftPreProcessorError("Regular expression cannot be processed since grep cannot be executed (not found)");
            }
        } else {
            if (input.find(key) != std::string::npos) {
                replace_all(input, key, value);
            }
        }
    }
}

// declarative preprocessing directives and symbol extraction
void PreProcess(std::vector<std::string> &file, defined_line_marker_t &defined_line_marker, uint64_t &org, const bool regex,
                std::map<std::string, std::string> &equ_replacement, const header_file_list_t &headers,
                source_file_c_style_definition_t &definition, const std::vector<std::string> &include_path)
{
    uint64_t line_number = 0;

    auto is_line_empty = [](std::string line)->bool {
        replace_all(line, " ", "");
        return line.empty();
    };

    auto is_line_marker_present = [&](const std::string & marker)->bool
    {
        for (const auto & line_marker : defined_line_marker) {
            if (line_marker.line_marker_name == marker) {
                return true;
            }
        }

        return false;
    };

    auto is_header_in_this_line = [&](const uint64_t line, std::vector < std::string > & header_file)->std::string
    {
        for (const auto & header : headers) {
            if (header.appearance_at_line_num == line) {
                header_file = header.content;
                return header.file_name;
            }
        }

        return "";
    };

    // discard all comments
    for (auto & line : file) {
        line = truncateAfterSemicolonOrHash(line);
        // remove '\t'
        replace_all(line, "\t", "    ");
    }

    // register all symbols
    auto markers = line_marker_register(file);

    // define symbols
    for (auto & marker : markers)
    {
        if (!is_line_marker_present(marker)) {
            defined_line_marker.emplace_back(line_marker_t {
                .line_marker_name = marker,
                .marker_position = 0,
                .is_defined = false,
                .referenced = false,
                .loc_it_appeared_in_cur_blk = {}
            });
        }
    }

    // process declarative directives
    for (auto & line : file)
    {
        line_number++;

        std::vector < std::string > header_file;
        if (const auto filename = is_header_in_this_line(line_number, header_file);
            !filename.empty())
        {
            try {
                header_file_list_t header_files;
                HeadProcess(header_file, definition, header_files, include_path);
                PreProcess(header_file,
                    defined_line_marker,
                    org,
                    regex,
                    equ_replacement,
                    header_files,
                    definition,
                    include_path);
            } catch (const SysdarftPreProcessorError & e) {
                throw SysdarftPreProcessorError("Error when processing file " + filename + ": " + e.what());
            }
        }

        line = truncateAfterSemicolonOrHash(line);
        if (is_line_empty(line)) {
            continue;
        }

        try {
            // search for each preprocessor pattern
            if (std::regex_match(line, org_pattern)) {
                process_org(line, org);
            } else if (std::regex_match(line, lab_pattern)) {
                process_lab(line, defined_line_marker);
            } else if (std::regex_match(line, equ_pattern)) {
                process_equ(line, equ_replacement);
            } else {
                break;
            }

            line.clear(); // clear preprocessor directives
        } catch (const std::exception & e) {
            throw SysdarftPreProcessorError("Line: " + std::to_string(line_number)
                + ": Error occurred when processing " + line + ": " + e.what());
        }
    }

    // equal replace
    for (auto & line : file) {
        sed_equ(line, equ_replacement, regex);
    }
}
