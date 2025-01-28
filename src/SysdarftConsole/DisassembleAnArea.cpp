/* DisassemblerAnArea.cpp
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
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

std::string insert_newlines_every_24(const std::string& input)
{
    constexpr std::size_t line_length = 24;
    if (input.length() < line_length) {
        return input;
    }

    std::string result;
    result.reserve(input.size() + input.size() / line_length);
    // Reserve space considering added newlines

    for (std::size_t i = 0; i < input.size(); ++i)
    {
        if (i > 0 && i % line_length == 0) {
            result.push_back('\n');
        }
        result.push_back(input[i]);
    }

    return result;
}

std::string linear_binary_to_string(const std::vector<uint8_t> & data)
{
    std::stringstream ss;
    for (const auto & c : data) {
        ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
           << static_cast<int>(c);
        ss << " ";
    }

    return ss.str();
}

std::string disassemble_code(std::vector < uint8_t > assembled_code,
    const uint64_t org, const std::map < uint64_t, std::string >& symbol_table)
{
    std::stringstream ret;
    auto assembled_code_reference = assembled_code;
    const std::regex _8bit_data(R"(.8bit_data <(.*)>)");
    const auto assembled_code_space = assembled_code.size();
    std::vector < std::vector < uint8_t > > bad_8bit_data;
    uint64_t bad_8bit_data_offset = 0;
    uint64_t concussive_nop_appearances = 0;
    uint64_t concussive_nop_appearances_offset = 0;

    auto clear_concussive_nop_appearances = [&]()->void
    {
        if (concussive_nop_appearances > 3)
        {
            ret << "\n ... PADDLING 0x00 APPEARED " << std::dec << concussive_nop_appearances << " TIMES SINCE "
                << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
                << concussive_nop_appearances_offset << "..." << std::endl << std::endl;
        }

        concussive_nop_appearances = 0;
        concussive_nop_appearances_offset = 0;
    };

    while (!assembled_code.empty())
    {
        std::vector < std::string > disassembled_code_literals;
        const auto current_pos = assembled_code_space - assembled_code.size() + org;
        const auto offset_before = assembled_code_space - assembled_code.size();

        decode_instruction(disassembled_code_literals, assembled_code);

        const auto offset_after = assembled_code_space - assembled_code.size();
        const auto decoded_code_literal_length = offset_after - offset_before;
        std::vector < uint8_t > decoded_literal_binary;

        decoded_literal_binary.resize(decoded_code_literal_length);
        std::memcpy(decoded_literal_binary.data(),
            assembled_code_reference.data() + offset_before,
            decoded_code_literal_length);

        // check if it's bad data
        // true if empty,
        // further code check is not performed but is indeed a bad data
        bool bad_data = disassembled_code_literals.empty();
        for (const auto & line : disassembled_code_literals)
        {
            if (std::regex_search(line, _8bit_data)) {
                bad_data = true;
                break;
            }
        }

        // if bad_data
        if (bad_data)
        {
            if (bad_8bit_data.empty()) {
                bad_8bit_data_offset = current_pos; // record current position for later use
            }
            bad_8bit_data.emplace_back(decoded_literal_binary);
            clear_concussive_nop_appearances();
            continue;
        }

        // made it here, check if there is any preceding bad data
        if (!bad_8bit_data.empty())
        {
            // unify data
            std::vector < uint8_t > unified_bad_data;
            for (const auto & line : bad_8bit_data) {
                unified_bad_data.insert(unified_bad_data.end(), line.begin(), line.end());
            }

            if (symbol_table.contains(bad_8bit_data_offset)) {
                ret << std::endl << std::endl << "<" << symbol_table.at(bad_8bit_data_offset) << "> :" << std::endl;
            }
            auto dumped = xxd_like_dump(bad_8bit_data_offset, unified_bad_data);
            ret << dumped << std::endl;
            bad_8bit_data.clear();
            bad_8bit_data_offset = 0;
        }

        if (!disassembled_code_literals.empty() && disassembled_code_literals[0] == "NOP")
        {
            if (concussive_nop_appearances == 0) {
                concussive_nop_appearances_offset = current_pos;
            }

            concussive_nop_appearances++;
        } else {
            clear_concussive_nop_appearances();
        }

        if (symbol_table.contains(current_pos)) {
            ret << std::endl << std::endl << "<" << symbol_table.at(current_pos) << "> :" << std::endl;
        }

        if (concussive_nop_appearances <= 3)
        {
            // write current offset
            ret << std::hex << std::setfill('0') << std::setw(16) << std::uppercase << current_pos << ": ";

            // first, we need a binary view of the data

            // then, we need to bend it into multiple lines
            if (std::string disassembled_data_literal = linear_binary_to_string(decoded_literal_binary);
                disassembled_data_literal.size() > 24)
            {
                disassembled_data_literal = insert_newlines_every_24(disassembled_data_literal);
                // insert instruction after first '\n'
                auto pos = disassembled_data_literal.find_first_of('\n');
                disassembled_data_literal.insert(pos, "    " + disassembled_code_literals[0]);
                replace_all(disassembled_data_literal, "\n", "\n" + std::string(18, ' '));
                ret << disassembled_data_literal << std::endl;
            } else {
                ret << disassembled_data_literal << std::string(28 - disassembled_data_literal.size(), ' ')
                    << disassembled_code_literals[0] << std::endl;
            }
        }
    }

    if (concussive_nop_appearances != 0) {
        clear_concussive_nop_appearances();
    }

    return ret.str();
}

void RemoteDebugServer::crow_setup_disassemble_an_area()
{
    CROW_ROUTE(JSONBackend, "/DisassembleMemory").methods(crow::HTTPMethod::GET)
        ([this](const crow::request& req)
        {
            // Create a JSON object using nlohmann/json
            json response;
            response["Version"] = SYSDARFT_VERSION;
            const auto timeNow = std::chrono::system_clock::now();
            response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                timeNow.time_since_epoch()).count());

            if (!breakpoint_triggered) {
                response["Result"] = "Still running";
                return crow::response(400, response.dump());
            }

            std::string bodyString = req.body;
            json clientJson;
            try {
                clientJson = json::parse(bodyString);
            } catch (const std::exception&) {
                response["Result"] = "Invalid Argument";
                return crow::response(400, response.dump());
            }

            if  (!clientJson.contains("Begin"))
            {
                response["Result"] = "Invalid Argument (Missing begin)";
                return crow::response(400, response.dump());
            }

            if  (!clientJson.contains("End"))
            {
                response["Result"] = "Invalid Argument (Missing end)";
                return crow::response(400, response.dump());
            }

            try {
                std::string begin = clientJson["Begin"];
                std::string end = clientJson["End"];
                remove_spaces(begin);
                remove_spaces(end);

                process_base16(begin);
                process_base16(end);

                if (begin.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                if (end.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                const uint64_t beginAddress = std::stoull(begin);
                const uint64_t endAddress = std::stoull(end);

                if (beginAddress >= endAddress) {
                    throw std::invalid_argument("Invalid Expression");
                }

                std::vector<std::string> instructions;
                const uint64_t offset = std::min(beginAddress, CPUInstance.SystemTotalMemory());
                const uint64_t length = std::min<uint64_t>(endAddress - beginAddress, CPUInstance.SystemTotalMemory() - offset);
                std::vector<uint8_t> buffer;
                buffer.resize(length);
                CPUInstance.read_memory(offset, (char*)buffer.data(), length);
                response["Result"] = disassemble_code(buffer, beginAddress);
            } catch (const std::exception & e) {
                response["Result"] = "Actionable Expression Error: " + std::string(e.what());
                return crow::response(400, response.dump());
            }

            return crow::response{response.dump()};
        });
}
