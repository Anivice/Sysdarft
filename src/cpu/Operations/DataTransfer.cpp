/* DataTransfer.cpp
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

void SysdarftCPUInstructionExecutor::mov(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto src = WidthAndOperands.second[1].get_val();
    WidthAndOperands.second[0].set_val(src);
}

void SysdarftCPUInstructionExecutor::xchg(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto operand2 = WidthAndOperands.second[1].get_val();

    // exchange
    WidthAndOperands.second[0].set_val(operand2);
    WidthAndOperands.second[1].set_val(operand1);
}

void SysdarftCPUInstructionExecutor::push(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  push_stack<uint8_t>(operand1);  break;
    case _16bit_prefix: push_stack<uint16_t>(operand1); break;
    case _32bit_prefix: push_stack<uint32_t>(operand1); break;
    case _64bit_prefix: push_stack<uint64_t>(operand1); break;
    default: throw IllegalInstruction("Unknown width");
    }
}

void SysdarftCPUInstructionExecutor::pop(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    uint64_t val = 0;
    switch (WidthAndOperands.first) {
    case _8bit_prefix:  val = pop_stack<uint8_t>();  break;
    case _16bit_prefix: val = pop_stack<uint16_t>(); break;
    case _32bit_prefix: val = pop_stack<uint32_t>(); break;
    case _64bit_prefix: val = pop_stack<uint64_t>(); break;
    default: throw IllegalInstruction("Unknown width");
    }

    WidthAndOperands.second[0].set_val(val);
}

struct pushall_data
{
    uint64_t FER0;
    uint64_t FER1;
    uint64_t FER2;
    uint64_t FER3;
    uint64_t FER4;
    uint64_t FER5;
    uint64_t FER6;
    uint64_t FER7;
    uint64_t FER8;
    uint64_t FER9;
    uint64_t FER10;
    uint64_t FER11;
    uint64_t FER12;
    uint64_t FER13;
    uint64_t FER14;
    uint64_t FER15;
    decltype(sysdarft_register_t::FlagRegister) FG;
    uint64_t SB;
    uint64_t SP;
    uint64_t DB;
    uint64_t DP;
    uint64_t EB;
    uint64_t EP;
    uint64_t CPS;
};

void SysdarftCPUInstructionExecutor::pushall(__uint128_t, WidthAndOperandsType &)
{
    const pushall_data data = {
        .FER0 = SysdarftRegister::load<FullyExtendedRegisterType, 0>(),
        .FER1 = SysdarftRegister::load<FullyExtendedRegisterType, 1>(),
        .FER2 = SysdarftRegister::load<FullyExtendedRegisterType, 2>(),
        .FER3 = SysdarftRegister::load<FullyExtendedRegisterType, 3>(),
        .FER4 = SysdarftRegister::load<FullyExtendedRegisterType, 4>(),
        .FER5 = SysdarftRegister::load<FullyExtendedRegisterType, 5>(),
        .FER6 = SysdarftRegister::load<FullyExtendedRegisterType, 6>(),
        .FER7 = SysdarftRegister::load<FullyExtendedRegisterType, 7>(),
        .FER8 = SysdarftRegister::load<FullyExtendedRegisterType, 8>(),
        .FER9 = SysdarftRegister::load<FullyExtendedRegisterType, 9>(),
        .FER10 = SysdarftRegister::load<FullyExtendedRegisterType, 10>(),
        .FER11 = SysdarftRegister::load<FullyExtendedRegisterType, 11>(),
        .FER12 = SysdarftRegister::load<FullyExtendedRegisterType, 12>(),
        .FER13 = SysdarftRegister::load<FullyExtendedRegisterType, 13>(),
        .FER14 = SysdarftRegister::load<FullyExtendedRegisterType, 14>(),
        .FER15 = SysdarftRegister::load<FullyExtendedRegisterType, 15>(),
        .FG = SysdarftRegister::load<FlagRegisterType>(),
        .SB = SysdarftRegister::load<StackBaseType>(),
        .SP = SysdarftRegister::load<StackPointerType>(),
        .DB = SysdarftRegister::load<DataBaseType>(),
        .DP = SysdarftRegister::load<DataPointerType>(),
        .EB = SysdarftRegister::load<ExtendedBaseType>(),
        .EP = SysdarftRegister::load<ExtendedPointerType>(),
        .CPS = SysdarftRegister::load<CurrentProcedureStackPreservationSpaceType>(),
    };

    push_stack(data);
}

void SysdarftCPUInstructionExecutor::popall(__uint128_t, WidthAndOperandsType &)
{
    auto [FER0,
        FER1,
        FER2,
        FER3,
        FER4,
        FER5,
        FER6,
        FER7,
        FER8,
        FER9,
        FER10,
        FER11,
        FER12,
        FER13,
        FER14,
        FER15,
        FG,
        SB,
        SP,
        DB,
        DP,
        EB,
        EP,
        CPS] = pop_stack<pushall_data>();

    SysdarftRegister::store<FullyExtendedRegisterType, 0>(FER0);
    SysdarftRegister::store<FullyExtendedRegisterType, 1>(FER1);
    SysdarftRegister::store<FullyExtendedRegisterType, 2>(FER2);
    SysdarftRegister::store<FullyExtendedRegisterType, 3>(FER3);
    SysdarftRegister::store<FullyExtendedRegisterType, 4>(FER4);
    SysdarftRegister::store<FullyExtendedRegisterType, 5>(FER5);
    SysdarftRegister::store<FullyExtendedRegisterType, 6>(FER6);
    SysdarftRegister::store<FullyExtendedRegisterType, 7>(FER7);
    SysdarftRegister::store<FullyExtendedRegisterType, 8>(FER8);
    SysdarftRegister::store<FullyExtendedRegisterType, 9>(FER9);
    SysdarftRegister::store<FullyExtendedRegisterType, 10>(FER10);
    SysdarftRegister::store<FullyExtendedRegisterType, 11>(FER11);
    SysdarftRegister::store<FullyExtendedRegisterType, 12>(FER12);
    SysdarftRegister::store<FullyExtendedRegisterType, 13>(FER13);
    SysdarftRegister::store<FullyExtendedRegisterType, 14>(FER14);
    SysdarftRegister::store<FullyExtendedRegisterType, 15>(FER15);
    SysdarftRegister::store<FlagRegisterType>(FG);
    SysdarftRegister::store<StackBaseType>(SB);
    SysdarftRegister::store<StackPointerType>(SP);
    SysdarftRegister::store<DataBaseType>(DB);
    SysdarftRegister::store<DataPointerType>(DP);
    SysdarftRegister::store<ExtendedBaseType>(EB);
    SysdarftRegister::store<ExtendedPointerType>(EP);
    SysdarftRegister::store<CurrentProcedureStackPreservationSpaceType>(CPS);
}

void SysdarftCPUInstructionExecutor::enter(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto operand1 = WidthAndOperands.second[0].get_val();
    const auto SP = SysdarftRegister::load<StackPointerType>();
    SysdarftRegister::store<CurrentProcedureStackPreservationSpaceType>(operand1);
    SysdarftRegister::store<StackPointerType>(SP - operand1);
}

void SysdarftCPUInstructionExecutor::leave(__uint128_t, WidthAndOperandsType &)
{
    const auto SP = SysdarftRegister::load<StackPointerType>();
    const auto CPS = SysdarftRegister::load<CurrentProcedureStackPreservationSpaceType>();
    SysdarftRegister::store<StackPointerType>(SP + CPS);
    SysdarftRegister::store<CurrentProcedureStackPreservationSpaceType>(0);
}

void SysdarftCPUInstructionExecutor::movs(__uint128_t, WidthAndOperandsType &)
{
    const uint64_t dest = SysdarftRegister::load<DataPointerType>() + SysdarftRegister::load<DataBaseType>();
    const uint64_t src = SysdarftRegister::load<ExtendedPointerType>() + SysdarftRegister::load<ExtendedBaseType>();
    const uint64_t count = SysdarftRegister::load<FullyExtendedRegisterType, 3>();
    const auto buffer = new char [count];
    SysdarftCPUMemoryAccess::read_memory(src, buffer, count);
    SysdarftCPUMemoryAccess::write_memory(dest, buffer, count);
    delete[] buffer;
}

void SysdarftCPUInstructionExecutor::lea(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const uint64_t effective_addr = WidthAndOperands.second[1].get_effective_addr();
    WidthAndOperands.second[0].set_val(effective_addr);
}
