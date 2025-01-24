/* Watcher.cpp
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

void RemoteDebugServer::crow_setup_watcher()
{
    CROW_ROUTE(JSONBackend, "/Watcher").methods(crow::HTTPMethod::POST)
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
            response["Result"] = "Invalid Argument";
            return crow::response(400, response.dump());
        }

        std::vector<uint8_t> target_byte_code;
        if (!clientJson.contains("Expression")) {
            response["Result"] = "Invalid Argument: Missing expression!";
            return crow::response(400, response.dump());
        }

        try {
            std::string expression = clientJson["Expression"];
            remove_spaces(expression);
            capitalization(expression);
            replace_all(expression, "<", "");
            replace_all(expression, ">", "");
            encode_target(target_byte_code, expression);
        } catch (const std::exception& e) {
            response["Result"] = "Invalid Argument: " + std::string(e.what());
            return crow::response(400, response.dump());
        }

        std::lock_guard lock(g_br_list_access_mutex);
        watch_list.emplace_back(target_byte_code, 0);
        response["Result"] = "Success";

        return crow::response{response.dump()};
    });
}
