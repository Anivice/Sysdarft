#ifndef SYSDARFTINSTRUCTIONEXEC_H
#define SYSDARFTINSTRUCTIONEXEC_H

#include <SysdarftRegister.h>
#include <SysdarftMemory.h>

class SysdarftCPUInstructionExecutor
{
protected:
    std::map <uint8_t /* opcode */, void (SysdarftCPUInstructionExecutor::*)(__uint128_t) /* this->Method */> ExecutorMap;
    void nop(__uint128_t timestamp);
    void add(__uint128_t timestamp);

    SysdarftCPUInstructionExecutor();
};

#endif //SYSDARFTINSTRUCTIONEXEC_H
