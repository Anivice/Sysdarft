#include <SysdarftInstructionExec.h>
#include <InstructionSet.h>

SysdarftCPUInstructionExecutor::SysdarftCPUInstructionExecutor()
{
    ExecutorMap.emplace(OPCODE_NOP, &SysdarftCPUInstructionExecutor::nop);
}
