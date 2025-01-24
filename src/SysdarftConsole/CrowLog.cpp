/* CrowLog.cpp
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

void RemoteDebugServer::SysdarftLogHandler::log(std::string message, crow::LogLevel level)
{
    if (!log_available) {
        return;
    }

    const char *level_text = nullptr;
    switch (level) {
    case crow::LogLevel::Debug:
        level_text = "[DEBUG]";
        break;
    case crow::LogLevel::Info:
        level_text = "[INFO]";
        break;
    case crow::LogLevel::Warning:
        level_text = "[WARN]";
        break;
    case crow::LogLevel::Error:
        level_text = "[ERROR]";
        break;
    case crow::LogLevel::Critical:
        level_text = "[CRITICAL]";
        break;
    default:
        level_text = "[LOG]";
        break;
    }

    custom_stream << level_text << " " << message << std::endl;
}
