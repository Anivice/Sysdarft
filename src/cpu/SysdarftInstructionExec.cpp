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

    // Debug Handler
    bindBreakpointHandler(this, &SysdarftCPUInstructionExecutor::default_breakpoint_handler);
    bindIsBreakHere(this, &SysdarftCPUInstructionExecutor::default_is_break_here);
}

void SysdarftCPUInstructionExecutor::execute(const __uint128_t timestamp)
{
    auto [opcode, width, operands, literal]
        = SysdarftCPUInstructionDecoder::pop_instruction_from_ip_and_increase_ip();

    WidthAndOperandsType Arg = std::make_pair(width, operands);

    if (is_break_here()) {
        log("[CPU] Breakpoint reached!\n");
        breakpoint_handler(timestamp, opcode, Arg);
    } else {
        // FIXME: Mask FPU and Signed output
        log("[CPU] ", literal, "\n");
    }

    try {
        (this->*ExecutorMap.at(opcode))(timestamp, Arg);
    } catch (std::out_of_range&) {
        log("[CPU] Instruction `", literal, "` not implemented.\n");
    }
}
