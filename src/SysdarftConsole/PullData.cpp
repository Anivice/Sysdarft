/* PullData.cpp
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

void RemoteDebugServer::crow_setup_pull_data()
{
    CROW_ROUTE(JSONBackend, "/PullData").methods(crow::HTTPMethod::POST)
        ([this](const crow::request& req)
        {
            // Create a JSON object using nlohmann/json
            json response;
            response["Version"] = SYSDARFT_VERSION;
            const auto timeNow = std::chrono::system_clock::now();
            response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                timeNow.time_since_epoch()).count());

            std::string bodyString = req.body;
            json clientJson;
            try {
                clientJson = json::parse(bodyString);
            } catch (const std::exception&) {
                throw std::invalid_argument("Invalid Expression");
            }

            if  (!clientJson.contains("Begin") || !clientJson.contains("End"))
            {
                throw std::invalid_argument("Invalid Expression");
            }

            try {
                std::string begin = clientJson["Begin"];
                std::string end = clientJson["End"];

                remove_spaces(begin);
                remove_spaces(end);
                process_base16(begin);
                process_base16(end);

                if (begin.empty() || end.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                const auto begin_num = std::stoull(begin);
                const auto end_num = std::stoull(end);

                if (begin_num >= end_num) {
                     throw std::invalid_argument("Invalid Expression");
                 }

                std::vector < uint8_t > dump;
                const auto dump_size = end_num - begin_num + 1;
                dump.resize(dump_size);
                CPUInstance.read_memory(begin_num, (char*)dump.data(), dump_size);
                const auto dumped = xxd_like_dump(begin_num, dump);
                response["Result"] = dumped;
            } catch (const std::exception & e) {
                response["Result"] = "Actionable Expression Error: " + std::string(e.what());
                return crow::response(400, response.dump());
            }

            return crow::response{response.dump()};
        });
}
