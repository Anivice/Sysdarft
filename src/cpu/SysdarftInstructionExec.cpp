#include <SysdarftInstructionExec.h>
#include <InstructionSet.h>

SysdarftCPUInstructionExecutor::SysdarftCPUInstructionExecutor()
{
    // Misc
    make_instruction_execution_procedure(OPCODE_NOP, &SysdarftCPUInstructionExecutor::nop);

    // Arithmetic
    make_instruction_execution_procedure(OPCODE_ADD, &SysdarftCPUInstructionExecutor::add);

    // Data Transfer
    make_instruction_execution_procedure(OPCODE_MOV, &SysdarftCPUInstructionExecutor::mov);
}

void SysdarftCPUInstructionExecutor::execute(__uint128_t timestamp)
{
    auto [opcode, width, operands, literal]
        = SysdarftCPUInstructionDecoder::pop_instruction_from_ip_and_increase_ip();

    if (break_here) {
        // TODO: Debug output
    } else {
        // FIXME: Mask FPU and Signed output
        log("[CPU] ", literal, "\n");
    }

    WidthAndOperandsType Arg = std::make_pair(width, operands);

    try {
        (this->*ExecutorMap.at(opcode))(timestamp, Arg);
    } catch (std::out_of_range& e) {
        log("[CPU] Instruction `", literal, "` not implemented.\n");
    }
}
