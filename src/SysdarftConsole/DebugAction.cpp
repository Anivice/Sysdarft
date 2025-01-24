/* DebugAction.cpp
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

void RemoteDebugServer::at_breakpoint(__uint128_t,
    const uint64_t actual_ip,
    const uint8_t opcode,
    const SysdarftCPU::WidthAndOperandsType & args)
{
    this->actual_ip = actual_ip;
    this->opcode = opcode;
    this->args = &args;

    while (breakpoint_triggered)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    this->args = nullptr;
}
