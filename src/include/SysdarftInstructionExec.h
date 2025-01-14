#ifndef SYSDARFTINSTRUCTIONEXEC_H
#define SYSDARFTINSTRUCTIONEXEC_H

#include <any>
#include <SysdarftCPUDecoder.h>
#include <SysdarftIOHub.h>

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
        const auto StackNewLowerEnd = SP - sizeof(DataType);
        SysdarftCPUMemoryAccess::write_memory(StackNewLowerEnd + SB, (char*)&val, sizeof(DataType));
        SysdarftRegister::store<StackPointerType>(StackNewLowerEnd);
    }

    template < typename DataType >
    DataType pop_stack()
    {
        DataType val { };
        const auto SP = SysdarftRegister::load<StackPointerType>();
        const auto SB = SysdarftRegister::load<StackBaseType>();
        SysdarftCPUMemoryAccess::read_memory(SB + SP, (char*)&val, sizeof(DataType));
        SysdarftRegister::store<StackPointerType>(SP + sizeof(DataType));
        return val;
    }

protected:
    typedef std::pair < uint8_t /* width */, std::vector < OperandType > > WidthAndOperandsType;
    std::map <uint8_t /* opcode */,
        void (SysdarftCPUInstructionExecutor::*)(__uint128_t, WidthAndOperandsType &) /* method */ > ExecutorMap;

    void make_instruction_execution_procedure(uint8_t opcode,
        void (SysdarftCPUInstructionExecutor::*method)(__uint128_t, WidthAndOperandsType &))
    {
        ExecutorMap.emplace(opcode, method);
    }

    void show_context();
    bool default_is_break_here() { return false; }
    void default_breakpoint_handler(__uint128_t, uint8_t, const WidthAndOperandsType &) { }

    using IsBreakHereFn = std::function<bool()>;
    using BreakpointHandlerFn = std::function<void(__uint128_t, uint8_t, const WidthAndOperandsType &)>;

    IsBreakHereFn is_break_here;
    BreakpointHandlerFn breakpoint_handler;

    template < class InstanceType >
    void bindIsBreakHere(InstanceType* instance, bool (InstanceType::*memFunc)())
    {
        is_break_here = [instance, memFunc]() -> bool {
            return (instance->*memFunc)();
        };
    }

    template < class InstanceType >
    void bindBreakpointHandler(InstanceType* instance, void (InstanceType::*memFunc)(
        __uint128_t, uint8_t, const WidthAndOperandsType &))
    {
        breakpoint_handler = [instance, memFunc](__uint128_t val,
            uint8_t opcode,
            const WidthAndOperandsType & wapr)
        {
            (instance->*memFunc)(val, opcode, wapr);
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

    // io
    add_instruction_exec(in);
    add_instruction_exec(out);
    add_instruction_exec(ins);
    add_instruction_exec(outs);

protected:
    // initialization
    explicit SysdarftCPUInstructionExecutor(uint64_t memory);

    // general code execution
    void execute(__uint128_t timestamp);
};

#undef add_instruction_exec

#endif //SYSDARFTINSTRUCTIONEXEC_H
