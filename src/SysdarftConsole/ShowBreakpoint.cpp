/* ShowBreakpoint.cpp
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

void RemoteDebugServer::crow_setup_showBreakpoint()
{
    CROW_ROUTE(JSONBackend, "/ShowBreakpoint").methods(crow::HTTPMethod::GET)([this]()
    {
        json response;
        response["Version"] = SYSDARFT_VERSION;
        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());
        std::stringstream ss;

        ss << "[BREAKPOINT]" << std::endl;
        std::lock_guard lock(g_br_list_access_mutex);
        for (const auto& breakpoint : breakpoint_list)
        {
            ss  << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
                << breakpoint.first.first + breakpoint.first.second << ": ";
            for (const auto & e : breakpoint.second) {
                ss << e;
            }

            ss << std::endl;
        }

        ss << "[WATCHLIST]" << std::endl;
        for (const auto& watch : watch_list)
        {
            for (const auto & e : watch.first) {
                ss << e;
            }

            ss << std::endl;
        }

        ss << "[END]" << std::endl;

        response["Result"] = ss.str();
        return crow::response{response.dump()};
    });
}
