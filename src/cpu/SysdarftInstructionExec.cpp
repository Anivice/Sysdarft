#include <SysdarftInstructionExec.h>
#include <InstructionSet.h>

SysdarftCPUInstructionExecutor::SysdarftCPUInstructionExecutor()
{
    // Misc
    make_instruction_execution_procedure(OPCODE_NOP, &SysdarftCPUInstructionExecutor::nop);
    make_instruction_execution_procedure(OPCODE_HLT, &SysdarftCPUInstructionExecutor::hlt);

    // Arithmetic
    make_instruction_execution_procedure(OPCODE_ADD, &SysdarftCPUInstructionExecutor::add);
    make_instruction_execution_procedure(OPCODE_ADC, &SysdarftCPUInstructionExecutor::adc);
    make_instruction_execution_procedure(OPCODE_SUB, &SysdarftCPUInstructionExecutor::sub);
    make_instruction_execution_procedure(OPCODE_SBB, &SysdarftCPUInstructionExecutor::sbb);
    make_instruction_execution_procedure(OPCODE_IMUL, &SysdarftCPUInstructionExecutor::imul);
    make_instruction_execution_procedure(OPCODE_MUL, &SysdarftCPUInstructionExecutor::mul);
    make_instruction_execution_procedure(OPCODE_IDIV, &SysdarftCPUInstructionExecutor::idiv);
    make_instruction_execution_procedure(OPCODE_DIV, &SysdarftCPUInstructionExecutor::div);
    make_instruction_execution_procedure(OPCODE_NEG, &SysdarftCPUInstructionExecutor::neg);
    make_instruction_execution_procedure(OPCODE_CMP, &SysdarftCPUInstructionExecutor::cmp);

    // Data Transfer
    make_instruction_execution_procedure(OPCODE_MOV, &SysdarftCPUInstructionExecutor::mov);
    make_instruction_execution_procedure(OPCODE_XCHG, &SysdarftCPUInstructionExecutor::xchg);
    make_instruction_execution_procedure(OPCODE_PUSH, &SysdarftCPUInstructionExecutor::push);
    make_instruction_execution_procedure(OPCODE_POP, &SysdarftCPUInstructionExecutor::pop);
    make_instruction_execution_procedure(OPCODE_PUSHALL, &SysdarftCPUInstructionExecutor::pushall);
    make_instruction_execution_procedure(OPCODE_POPALL, &SysdarftCPUInstructionExecutor::popall);
    make_instruction_execution_procedure(OPCODE_ENTER, &SysdarftCPUInstructionExecutor::enter);
    make_instruction_execution_procedure(OPCODE_LEAVE, &SysdarftCPUInstructionExecutor::leave);
    make_instruction_execution_procedure(OPCODE_MOVS, &SysdarftCPUInstructionExecutor::movs);
    make_instruction_execution_procedure(OPCODE_LEA, &SysdarftCPUInstructionExecutor::lea);

    // Logic and Bitwise
    make_instruction_execution_procedure(OPCODE_AND, &SysdarftCPUInstructionExecutor::and_);
    make_instruction_execution_procedure(OPCODE_OR, &SysdarftCPUInstructionExecutor::or_);
    make_instruction_execution_procedure(OPCODE_XOR, &SysdarftCPUInstructionExecutor::xor_);
    make_instruction_execution_procedure(OPCODE_NOT, &SysdarftCPUInstructionExecutor::not_);
    make_instruction_execution_procedure(OPCODE_SHL, &SysdarftCPUInstructionExecutor::shl);
    make_instruction_execution_procedure(OPCODE_SHR, &SysdarftCPUInstructionExecutor::shr);
    make_instruction_execution_procedure(OPCODE_ROL, &SysdarftCPUInstructionExecutor::rol);
    make_instruction_execution_procedure(OPCODE_ROR, &SysdarftCPUInstructionExecutor::ror);
    make_instruction_execution_procedure(OPCODE_RCL, &SysdarftCPUInstructionExecutor::rcl);
    make_instruction_execution_procedure(OPCODE_RCR, &SysdarftCPUInstructionExecutor::rcr);

    // Controlflow
    make_instruction_execution_procedure(OPCODE_JMP, &SysdarftCPUInstructionExecutor::jmp);
    make_instruction_execution_procedure(OPCODE_CALL, &SysdarftCPUInstructionExecutor::call);
    make_instruction_execution_procedure(OPCODE_RET, &SysdarftCPUInstructionExecutor::ret);
    make_instruction_execution_procedure(OPCODE_JE, &SysdarftCPUInstructionExecutor::je);
    make_instruction_execution_procedure(OPCODE_JNE, &SysdarftCPUInstructionExecutor::jne);
    make_instruction_execution_procedure(OPCODE_JB, &SysdarftCPUInstructionExecutor::jb);
    make_instruction_execution_procedure(OPCODE_JL, &SysdarftCPUInstructionExecutor::jl);
    make_instruction_execution_procedure(OPCODE_JBE, &SysdarftCPUInstructionExecutor::jbe);
    make_instruction_execution_procedure(OPCODE_JLE, &SysdarftCPUInstructionExecutor::jle);
    make_instruction_execution_procedure(OPCODE_INT, &SysdarftCPUInstructionExecutor::int_);
    make_instruction_execution_procedure(OPCODE_INT3, &SysdarftCPUInstructionExecutor::int3);
    make_instruction_execution_procedure(OPCODE_IRET, &SysdarftCPUInstructionExecutor::iret);

    // IOH
    make_instruction_execution_procedure(OPCODE_IN, &SysdarftCPUInstructionExecutor::in);
    make_instruction_execution_procedure(OPCODE_OUT, &SysdarftCPUInstructionExecutor::out);
    make_instruction_execution_procedure(OPCODE_INS, &SysdarftCPUInstructionExecutor::ins);
    make_instruction_execution_procedure(OPCODE_OUTS, &SysdarftCPUInstructionExecutor::outs);

    // Debug Handler
    bindBreakpointHandler(this, &SysdarftCPUInstructionExecutor::default_breakpoint_handler);
    bindIsBreakHere(this, &SysdarftCPUInstructionExecutor::default_is_break_here);
}

void SysdarftCPUInstructionExecutor::execute(const __uint128_t timestamp)
{
    auto [opcode, width, operands, literal]
        = SysdarftCPUInstructionDecoder::pop_instruction_from_ip_and_increase_ip();

    WidthAndOperandsType Arg = std::make_pair(width, operands);

    log("[CPU] ", literal, "\n");

    if (hd_int_flag || is_break_here())
    {
        log("[CPU] Breakpoint reached!\n");
        hd_int_flag = false;
        breakpoint_handler(timestamp, opcode, Arg);
    }

    try {
        (this->*ExecutorMap.at(opcode))(timestamp, Arg);
    } catch (std::out_of_range&) {
        log("[CPU] Instruction `", literal, "` not implemented.\n");
    } catch (SysdarftDeviceIOError&) {
        do_interruption(0x02);
    } catch (std::exception& err) {
        std::string err_str = err.what();
        log("[CPU] Unexpected error:", err.what(), "\n");
        do_interruption(0x00);
    }
}
