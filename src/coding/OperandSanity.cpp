#include <EncodingDecoding.h>
#include <InstructionSet.h>

void throw_constant_error()
{
    throw InstructionExpressionError("Operands cannot be constants");
}

bool isInvalid64BitOperand(const parsed_target_t& operand)
{
    // Check if the operand is a register and matches the invalid cases
    const bool isInvalidRegister = (operand.TargetType == parsed_target_t::REGISTER) &&
                             ((operand.RegisterName.at(1) == 'F') ||
                              (operand.RegisterName == "%SB") ||
                              (operand.RegisterName == "%SP") ||
                              (operand.RegisterName == "%CB") ||
                              (operand.RegisterName == "%DB") ||
                              (operand.RegisterName == "%DP") ||
                              (operand.RegisterName == "%EB") ||
                              (operand.RegisterName == "%EP"));

    // Check if the operand is memory and has an invalid width
    const bool isInvalidMemory = (operand.TargetType == parsed_target_t::MEMORY) &&
                           (operand.memory.MemoryWidth == "64");

    // Constant is always 64 bit in width
    const bool isConstant = (operand.TargetType == parsed_target_t::CONSTANT);

    return isInvalidRegister || isInvalidMemory || isConstant;
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
        if (operands.at(1).TargetType == parsed_target_t::CONSTANT) {
            throw_constant_error();
        }
        break;
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
        if (operands.at(0).TargetType == parsed_target_t::CONSTANT ||
            operands.at(1).TargetType != parsed_target_t::MEMORY)
        {
            throw InstructionExpressionError(
                "LEA only accepts register or memory reference as its first operand, "
                "and memory reference as its second operand");
        }

        if (!isInvalid64BitOperand(operands.at(0)))
        {
            throw InstructionExpressionError("LEA operand width is inconsistent with width enforcement scheme (WES)");
        }
        break;
    case OPCODE_JMP:
    case OPCODE_CALL:
    case OPCODE_JE:
    case OPCODE_JNE:
    case OPCODE_JB:
    case OPCODE_JL:
    case OPCODE_JBE:
    case OPCODE_JLE:
    case OPCODE_JC:
    case OPCODE_JNC:
    case OPCODE_JO:
    case OPCODE_JNO:
    case OPCODE_LOOP:
        if (!(isInvalid64BitOperand(operands.at(0)) && isInvalid64BitOperand(operands.at(1))))
        {
            throw InstructionExpressionError(
                "Control Flow instruction operand width is inconsistent with width enforcement scheme (WES)");
        }
        break;
    default:;
    }
}
