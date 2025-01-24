/* MainCompile.cpp
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

#include <SysdarftMain.h>

void compile_to_binary(const std::vector< std::string > & source_files,
    const std::string & binary_filename,
    const bool regex)
{
    std::vector < std::vector < uint8_t > > binary_cct;
    for (const std::string & source_file : source_files)
    {
        try {

            std::fstream file(source_file, std::ios::in | std::ios::out);
            if (!file.is_open()) {
                throw SysdarftAssemblerError("Could not open file " + source_file);
            }

            std::vector < uint8_t > binary;
            CodeProcessing(binary, file, regex);

            file.close();
            binary_cct.emplace_back(binary);
        } catch (const std::exception & e) {
            throw SysdarftAssemblerError("Error when processing file " + source_file + ": " + e.what());
        }
    }

    try {
        std::ofstream file(binary_filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw SysdarftAssemblerError("Could not open file " + binary_filename);
        }

        for (const auto & cct : binary_cct) {
            file.write(reinterpret_cast<const char*>(cct.data()), static_cast<ssize_t>(cct.size()));
        }

        file.close();
    } catch (const std::exception & e) {
        throw SysdarftAssemblerError("Error when writing to output file " + binary_filename + ": " + e.what());
    }
}


void disassemble(const std::string & binary_filename, const uint64_t org)
{
    std::ifstream file(binary_filename, std::ios::in | std::ios::binary);
    std::vector < uint8_t > assembled_code, assembled_code_backup;
    auto file_size = std::filesystem::file_size(binary_filename);

    // read
    if (!file.is_open()) {
        throw SysdarftDisassemblerError("Could not open file " + binary_filename);
    }

    assembled_code.resize(file_size);
    file.read((char*)(assembled_code.data()), static_cast<ssize_t>(file_size));
    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + binary_filename);
    }

    file.close();

    std::cout << disassemble_code(assembled_code, org) << std::endl;

    // assembled_code_backup = assembled_code;
    //
    // const std::regex _8bit_data(R"(.8bit_data <(.*)>)");
    // const auto space = assembled_code.size();
    // std::vector < std::string > lines;
    // std::vector < std::pair < uint64_t, std::string > > bad_data;
    // auto process_bad_data = [&]()->void
    // {
    //     std::stringstream ss;
    //     std::stringstream dat;
    //
    //     ss << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
    //        << bad_data.front().first;
    //     ss << ": ";
    //
    //     dat << ".8bit_data <";
    //     for (auto & [location, data] : bad_data) {
    //         dat << " " << data << ",";
    //     }
    //     dat << " >";
    //     bad_data.clear();
    //
    //     lines.push_back(ss.str());
    //     lines.push_back(dat.str());
    //     lines.emplace_back("");
    // };
    //
    // auto ins_code = [&](const std::vector<uint8_t> & code)->std::string
    // {
    //     std::stringstream ss;
    //     for (const auto & c : code) {
    //         ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
    //            << static_cast<int>(c);
    //         ss << " ";
    //     }
    //
    //     return ss.str();
    // };
    //
    // while (!assembled_code.empty())
    // {
    //     std::stringstream off;
    //     std::vector < std::string > line;
    //     auto current_pos = space - assembled_code.size() + org;
    //     off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
    //         << current_pos;
    //
    //     auto offset_before = space - assembled_code.size();
    //     decode_instruction(line, assembled_code);
    //     auto offset_after = space - assembled_code.size();
    //
    //     auto code_length = offset_after - offset_before;
    //     std::vector < uint8_t > current_code;
    //     current_code.resize(code_length);
    //     std::memcpy(
    //         current_code.data(),
    //         assembled_code_backup.data() + offset_before,
    //         code_length);
    //
    //     if (!line.empty())
    //     {
    //         bool continue_flag = false;
    //         // if bad data
    //         for (const auto & ls : line)
    //         {
    //             if (std::smatch match; std::regex_match(ls, match, _8bit_data)) {
    //                 bad_data.emplace_back(current_pos, match[1].str());
    //                 continue_flag = true;
    //             }
    //         }
    //
    //         // bad, skip print
    //         if (continue_flag) {
    //             continue;
    //         }
    //
    //         // not a match, flash cache
    //         if (!bad_data.empty())
    //         {
    //             process_bad_data();
    //         }
    //
    //         off << ": ";
    //         lines.push_back(off.str());
    //         lines.push_back(ins_code(current_code));
    //         lines.push_back(line[0]);
    //         lines.emplace_back("");
    //     }
    // }
    //
    // if (!bad_data.empty())
    // {
    //     process_bad_data();
    // }
    //
    // auto it = lines.begin();
    // if (lines.size() < 3) {
    //     return;
    // }
    //
    // while (it != lines.end())
    // {
    //     // check DATA2 in [cur] [DATA] [DATA2]
    //     const auto is_8bit_data = (it + 2)->empty(); // if empty, then 8bit data, else, no
    //     if (!is_8bit_data)
    //     {
    //         const auto & off_marker = *it;
    //         auto ins_code_exp = *(it + 1);
    //         ins_code_exp = insert_newlines_every_24(ins_code_exp);
    //         const auto & instruction_statement = *(it + 2);
    //         std::string padding_before(18, ' ');
    //         int padding_after_len = static_cast<int>(24 - (ins_code_exp.size() - ins_code_exp.find_last_of('\n') - 1));
    //
    //         if (ins_code_exp.size() > 24)
    //         {
    //             // '00000000000C18D3: ', 18 bytes
    //             replace_all(ins_code_exp, "\n", "\n" + padding_before);
    //             std::string padding_after(padding_after_len, ' ');
    //             ins_code_exp += padding_after;
    //
    //             auto pos = ins_code_exp.find_first_of('\n');
    //             ins_code_exp.insert(pos, "   " + instruction_statement);
    //
    //             std::cout << off_marker << ins_code_exp << std::endl;
    //         } else {
    //             std::string padding_after(27 - ins_code_exp.size(), ' ');
    //             std::cout << off_marker << ins_code_exp << padding_after << instruction_statement << std::endl;
    //         }
    //
    //         it += 4;
    //     } else {
    //         std::cout << *it << *(it + 1) << std::endl;
    //         it += 3;
    //     }
    // }
}
