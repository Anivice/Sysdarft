#ifndef SYSDARFTINSTRUCTIONEXEC_H
#define SYSDARFTINSTRUCTIONEXEC_H

#include <any>
#include <SysdarftCPUDecoder.h>

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInstructionExecutor : public SysdarftCPUInstructionDecoder
{
private:
    uint64_t check_overflow(uint8_t BCDWidth, __uint128_t Value);

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

    // Misc
    void nop(__uint128_t, WidthAndOperandsType &);

    // Arithmetic
    void add(__uint128_t, WidthAndOperandsType &);

    // Data transfer
    void mov(__uint128_t, WidthAndOperandsType &);

    // initialization
    SysdarftCPUInstructionExecutor();

    // general code execution
    void execute(__uint128_t timestamp);
};

#endif //SYSDARFTINSTRUCTIONEXEC_H
