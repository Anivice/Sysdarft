#ifndef SYSDARFTINSTRUCTIONEXEC_H
#define SYSDARFTINSTRUCTIONEXEC_H

#include <any>
#include <SysdarftCPUDecoder.h>

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInstructionExecutor : public SysdarftCPUInstructionDecoder
{
protected:
    typedef std::pair < uint8_t /* width */, std::vector < OperandType > > WidthAndOperandsType;
    std::map <uint8_t /* opcode */,
        void (SysdarftCPUInstructionExecutor::*)(__uint128_t, WidthAndOperandsType &) /* method */ > ExecutorMap;

    void make_instruction_execution_procedure(uint8_t opcode,
        void (SysdarftCPUInstructionExecutor::*method)(__uint128_t, WidthAndOperandsType &))
    {
        ExecutorMap.emplace(opcode, method);
    }

    std::atomic<bool> break_here = false;

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
