/* SysdarftInstructionExec.h
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

#ifndef SYSDARFTINSTRUCTIONEXEC_H
#define SYSDARFTINSTRUCTIONEXEC_H

#include <any>
#include <SysdarftCPUDecoder.h>
#include <SysdarftIOHub.h>

// CPU subroutine request to abort the current instruction execution procedure dur to error
class SysdarftCPUSubroutineRequestToAbortTheCurrentInstructionExecutionProcedureDueToError final : SysdarftBaseError {
public:
    SysdarftCPUSubroutineRequestToAbortTheCurrentInstructionExecutionProcedureDueToError() :
        SysdarftBaseError("Sysdarft Instruction Subroutine Abort") { }
};

#define add_instruction_exec(name) void name(__uint128_t, WidthAndOperandsType &)

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInstructionExecutor : public SysdarftCPUInstructionDecoder, public SysdarftIOHub
{
private:
    uint64_t check_overflow(uint8_t BCDWidth, __uint128_t Value);
    uint64_t check_overflow_signed(uint8_t BCDWidth, __uint128_t Value);

    template < typename DataType >
    void push_stack(const DataType & val)
    {
        const auto SP = SysdarftRegister::load<StackPointerType>();
        const auto SB = SysdarftRegister::load<StackBaseType>();

        // Stack overflow
        if (SP < sizeof(DataType)) {
            throw StackOverflow();
        }

        const auto StackNewLowerEnd = SP - sizeof(DataType);

        try {
            SysdarftCPUMemoryAccess::write_memory(StackNewLowerEnd + SB, (char*)&val, sizeof(DataType));
        } catch (IllegalMemoryAccessException & ) {
            throw StackOverflow();
        }

        SysdarftRegister::store<StackPointerType>(StackNewLowerEnd);
    }

    template < typename DataType >
    DataType pop_stack()
    {
        DataType val { };
        const auto SP = SysdarftRegister::load<StackPointerType>();
        const auto SB = SysdarftRegister::load<StackBaseType>();

        try {
            SysdarftCPUMemoryAccess::read_memory(SB + SP, (char*)&val, sizeof(DataType));
        } catch (IllegalMemoryAccessException & ) {
            throw StackOverflow();
        }

        SysdarftRegister::store<StackPointerType>(SP + sizeof(DataType));
        return val;
    }

public:
    typedef std::pair < uint8_t /* width */, std::vector < OperandType > > WidthAndOperandsType;

protected:
    std::map <uint8_t /* opcode */,
        void (SysdarftCPUInstructionExecutor::*)(__uint128_t, WidthAndOperandsType &) /* method */ > ExecutorMap;

    void make_instruction_execution_procedure(uint8_t opcode,
        void (SysdarftCPUInstructionExecutor::*method)(__uint128_t, WidthAndOperandsType &))
    {
        ExecutorMap.emplace(opcode, method);
    }

    void show_context();
    bool default_is_break_here(__uint128_t) { return false; }
    void default_breakpoint_handler(__uint128_t, uint64_t, uint8_t, const WidthAndOperandsType &) { }

    using IsBreakHereFn = std::function<bool(__uint128_t)>;
    using BreakpointHandlerFn = std::function<void(__uint128_t, uint64_t, uint8_t, const WidthAndOperandsType &)>;

    IsBreakHereFn is_break_here;
    BreakpointHandlerFn breakpoint_handler;

public:
    template < class InstanceType >
    void bindIsBreakHere(InstanceType* instance, bool (InstanceType::*memFunc)(__uint128_t))
    {
        is_break_here = [instance, memFunc](__uint128_t timestamp) -> bool {
            return (instance->*memFunc)(timestamp);
        };
    }

    template < class InstanceType >
    void bindBreakpointHandler(InstanceType* instance, void (InstanceType::*memFunc)(
        __uint128_t, uint64_t, uint8_t, const WidthAndOperandsType &))
    {
        breakpoint_handler = [instance, memFunc](__uint128_t val, uint64_t ip,
            uint8_t opcode,
            const WidthAndOperandsType & wapr)
        {
            (instance->*memFunc)(val, ip, opcode, wapr);
        };
    }

private:
    // Misc
    add_instruction_exec(nop);
    add_instruction_exec(hlt);
    add_instruction_exec(igni);
    add_instruction_exec(alwi);

    // Arithmetic
    add_instruction_exec(add);
    add_instruction_exec(adc);
    add_instruction_exec(sub);
    add_instruction_exec(sbb);
    add_instruction_exec(imul);
    add_instruction_exec(mul);
    add_instruction_exec(idiv);
    add_instruction_exec(div);
    add_instruction_exec(neg);
    add_instruction_exec(cmp);
    add_instruction_exec(inc);
    add_instruction_exec(dec);

    // Data Transfer
    add_instruction_exec(mov);
    add_instruction_exec(xchg);
    add_instruction_exec(push);
    add_instruction_exec(pop);
    add_instruction_exec(pushall);
    add_instruction_exec(popall);
    add_instruction_exec(enter);
    add_instruction_exec(leave);
    add_instruction_exec(movs);
    add_instruction_exec(lea);

    // Logic and Bitwise
    add_instruction_exec(and_);
    add_instruction_exec(or_);
    add_instruction_exec(xor_);
    add_instruction_exec(not_);
    add_instruction_exec(shl);
    add_instruction_exec(shr);
    add_instruction_exec(rol);
    add_instruction_exec(ror);
    add_instruction_exec(rcl);
    add_instruction_exec(rcr);

    // Control Flow
    add_instruction_exec(jmp);
    add_instruction_exec(call);
    add_instruction_exec(ret);
    add_instruction_exec(je);
    add_instruction_exec(jne);
    add_instruction_exec(jb);
    add_instruction_exec(jl);
    add_instruction_exec(jbe);
    add_instruction_exec(jle);
    add_instruction_exec(int_);
    add_instruction_exec(int3);
    add_instruction_exec(iret);
    add_instruction_exec(jc);
    add_instruction_exec(jnc);
    add_instruction_exec(jo);
    add_instruction_exec(jno);
    add_instruction_exec(loop);

    // IO
    add_instruction_exec(in);
    add_instruction_exec(out);
    add_instruction_exec(ins);
    add_instruction_exec(outs);

protected:
    // initialization
    explicit SysdarftCPUInstructionExecutor(uint64_t memory, const std::string & font_name);

    // general code execution
    void execute(__uint128_t timestamp);
};

#undef add_instruction_exec

#endif //SYSDARFTINSTRUCTIONEXEC_H
