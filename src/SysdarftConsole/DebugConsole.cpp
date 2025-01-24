/* DebugConsole.cpp
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

bool RemoteDebugServer::if_breakpoint(__uint128_t)
{
    try {
        breakpoint_triggered = false;

        if (stepi) {
            breakpoint_triggered = true;
            return true;
        }

        if (manual_stop)
        {
            breakpoint_triggered = true;
            manual_stop = false;
            return true;
        }

        const auto CB = CPUInstance.load<CodeBaseType>();
        const auto IP = CPUInstance.load<InstructionPointerType>();

        // halt system at startup, which is exactly where the start of BIOS is located
        if (!skip_bios_ip_check && (IP + CB == BIOS_START)) {
            skip_bios_ip_check = true; // skip next IP+CB == BIOS scenario
            breakpoint_triggered = true;
            return true;
        }

        std::lock_guard lock(g_br_list_access_mutex);
        for (const auto & [breakpoint, condition] :
            breakpoint_list)
        {
            if ((CB + IP) == (breakpoint.first + breakpoint.second))
            {
                if (is_condition_met(condition)) {
                    breakpoint_triggered = true;
                    return true;
                }
            }
        }

        bool is_hit = false;
        for (auto & watch : watch_list)
        {
            if (const uint64_t data_this_time = absolute_target_access(watch.first);
                data_this_time != watch.second)
            {
                watch.second = data_this_time;
                is_hit = true;
            }
        }

        breakpoint_triggered = is_hit;
        return is_hit;
    } catch (std::exception &) {
        return true;
    }
}
