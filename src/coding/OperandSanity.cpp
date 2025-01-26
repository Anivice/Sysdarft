#include <EncodingDecoding.h>
#include <InstructionSet.h>

void throw_constant_error()
{
    throw SysdarftAssemblerError("Operands cannot be constants");
}

void OperandSanityCheck(const uint8_t opcode, const std::vector < parsed_target_t > & operands)
{
    // Ensure the operand cannot be a constant when a writing to it
    switch (opcode) {
    case OPCODE_XCHG:
        if (operands.at(0).TargetType == parsed_target_t::CONSTANT
            || operands.at(1).TargetType == parsed_target_t::CONSTANT)
        {
            throw_constant_error();
        }
        break;
    case OPCODE_IN:
    case OPCODE_MOV:
    case OPCODE_POP:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_NOT:
    case OPCODE_SHL:
    case OPCODE_SHR:
    case OPCODE_ROL:
    case OPCODE_ROR:
    case OPCODE_RCL:
    case OPCODE_RCR:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_ADC:
    case OPCODE_SBB:
    case OPCODE_NEG:
    case OPCODE_INC:
    case OPCODE_DEC:
        if (operands.at(0).TargetType == parsed_target_t::CONSTANT) {
            throw_constant_error();
        }
        break;
    case OPCODE_LEA:
        if (operands.at(0).TargetType == parsed_target_t::CONSTANT &&
            operands.at(1).TargetType != parsed_target_t::MEMORY) {
            throw SysdarftAssemblerError(
                "LEA only accepts register or memory reference as its first operand, "
                "and memory reference as its second operand");
        }
        break;
    default:;
    }
}
