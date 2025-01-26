/* IOH.cpp
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

#include <SysdarftInstructionExec.h>

void SysdarftCPUInstructionExecutor::in(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto & port = Operands.second[0].get_val();
    auto && buffer = SysdarftIOHub::ins(port);
    const auto & data = buffer.pop<uint64_t>();
    Operands.second[1].set_val(data);
}

void SysdarftCPUInstructionExecutor::out(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto & port = Operands.second[0].get_val();
    const auto & data = Operands.second[1].get_val();

    ControllerDataStream buffer;
    buffer.push(data);
    SysdarftIOHub::outs(port, buffer);
}

void SysdarftCPUInstructionExecutor::ins(__uint128_t, WidthAndOperandsType & Operands)
{
    const auto DB = SysdarftRegister::load<DataBaseType>();
    const auto DP = SysdarftRegister::load<DataPointerType>();
    const auto CX = SysdarftRegister::load<FullyExtendedRegisterType, 3>();
    const auto & port = Operands.second[0].get_val();

    auto & buffer = SysdarftIOHub::ins(port);
    if (buffer.getSize() != CX) {
        throw SysdarftDeviceIOError("IO data length mismatch");
    }
    std::vector<uint8_t> wbuf = buffer.getObject();
    buffer.clear();
    SysdarftCPUMemoryAccess::write_memory(DB + DP, (char*)wbuf.data(), CX);
}

void SysdarftCPUInstructionExecutor::outs(__uint128_t, WidthAndOperandsType & Operands)
{
    ControllerDataStream buffer;
    const auto DB = SysdarftRegister::load<DataBaseType>();
    const auto DP = SysdarftRegister::load<DataPointerType>();
    const auto CX = SysdarftRegister::load<FullyExtendedRegisterType, 3>();
    const auto & port = Operands.second[0].get_val();

    std::vector<uint8_t> wbuf;
    wbuf.resize(CX);

    SysdarftCPUMemoryAccess::read_memory(DB + DP, (char*)wbuf.data(), CX);
    buffer.insert(wbuf);
    SysdarftIOHub::outs(port, buffer);
}
