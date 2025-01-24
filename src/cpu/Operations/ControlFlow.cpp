/* ControlFlow.cpp
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

void SysdarftCPUInstructionExecutor::jmp(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const uint64_t addr_base = WidthAndOperands.second[0].get_val();
    const uint64_t ip = WidthAndOperands.second[1].get_val();
    SysdarftRegister::store<CodeBaseType>(addr_base);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::call(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const auto ori_cb = SysdarftRegister::load<CodeBaseType>();
    const auto ori_ip = SysdarftRegister::load<InstructionPointerType>();

    push_stack(ori_cb);
    push_stack(ori_ip);

    const uint64_t addr_base = WidthAndOperands.second[0].get_val();
    const uint64_t ip = WidthAndOperands.second[1].get_val();
    SysdarftRegister::store<CodeBaseType>(addr_base);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::ret(__uint128_t, WidthAndOperandsType &)
{
    const auto ip = pop_stack<uint64_t>();
    const auto cb = pop_stack<uint64_t>();
    SysdarftRegister::store<CodeBaseType>(cb);
    SysdarftRegister::store<InstructionPointerType>(ip);
}

void SysdarftCPUInstructionExecutor::je(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jne(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (!SysdarftRegister::load<FlagRegisterType>().Equal)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jb(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().LargerThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jl(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().LessThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jbe(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal || SysdarftRegister::load<FlagRegisterType>().LargerThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jle(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Equal || SysdarftRegister::load<FlagRegisterType>().LessThan)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::int_(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    const uint64_t code = WidthAndOperands.second[0].get_val();
    SysdarftCPUInterruption::do_interruption(code);
}

void SysdarftCPUInstructionExecutor::int3(__uint128_t, WidthAndOperandsType &)
{
    SysdarftCPUInterruption::do_interruption(0x03);
}

void SysdarftCPUInstructionExecutor::iret(__uint128_t, WidthAndOperandsType &)
{
    SysdarftCPUInterruption::do_iret();
}

void SysdarftCPUInstructionExecutor::jc(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Carry)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jnc(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (!SysdarftRegister::load<FlagRegisterType>().Carry)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jo(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (SysdarftRegister::load<FlagRegisterType>().Overflow)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::jno(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    if (!SysdarftRegister::load<FlagRegisterType>().Overflow)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
    }
}

void SysdarftCPUInstructionExecutor::loop(__uint128_t, WidthAndOperandsType & WidthAndOperands)
{
    // jump if %fer3 != 0
    if (const auto cx = SysdarftRegister::load<FullyExtendedRegisterType, 3>() - 1;
        cx != 0)
    {
        const uint64_t addr_base = WidthAndOperands.second[0].get_val();
        const uint64_t ip = WidthAndOperands.second[1].get_val();
        SysdarftRegister::store<CodeBaseType>(addr_base);
        SysdarftRegister::store<InstructionPointerType>(ip);
        SysdarftRegister::store<FullyExtendedRegisterType, 3>(cx);
    }
}
