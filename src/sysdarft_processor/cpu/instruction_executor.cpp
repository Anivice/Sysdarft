#include <cpu.h>
#include <cstdint>

std::mutex interruption_vector_address_table_mutex_;
std::map < uint64_t /* Interruption number */, uint64_t /* Interruption vector address */>
    interruption_vector_address_table;

class __initialize_vector__ {
public:
    __initialize_vector__()
    {
        // Define parameters
        constexpr size_t TOTAL_ENTRIES = 512;

        // Initialize the table
        std::lock_guard lock(interruption_vector_address_table_mutex_);
        for (uint64_t i = 0; i < TOTAL_ENTRIES; i++)
        {
            constexpr uint64_t INCREMENT = 8;
            constexpr uint64_t START_ADDRESS = 0xA0000;
            uint64_t address = START_ADDRESS + i * INCREMENT;
            interruption_vector_address_table.insert(std::make_pair(i, address));
        }
    }
} __initialize_vector_instance__;



// Sample operation function
void processor::operation(const __uint128_t timestamp)
{
    if (is_at_breakpoint) {
        output_debug_info();
        breakpoint_handler.load()(timestamp, Registers, Memory);
    }

    switch (const auto opcode = pop<64>())
    {
    case OPCODE_NOP: InstructionExecutor.nop(timestamp); break;
    case OPCODE_ADD: InstructionExecutor.add(timestamp); break;
    case OPCODE_ADC: InstructionExecutor.adc(timestamp); break;
    case OPCODE_SUB: InstructionExecutor.sub(timestamp); break;
    case OPCODE_SBB: InstructionExecutor.sbb(timestamp); break;
    case OPCODE_IMUL: InstructionExecutor.imul(timestamp); break;
    case OPCODE_MUL: InstructionExecutor.mul(timestamp); break;
    case OPCODE_IDIV: InstructionExecutor.idiv(timestamp); break;
    case OPCODE_DIV: InstructionExecutor.div(timestamp); break;
    case OPCODE_NEG: InstructionExecutor.neg(timestamp); break;
    case OPCODE_CMP: InstructionExecutor.cmp(timestamp); break;

    case OPCODE_AND: InstructionExecutor.and_(timestamp); break;
    case OPCODE_OR: InstructionExecutor.or_(timestamp); break;
    case OPCODE_XOR: InstructionExecutor.xor_(timestamp); break;
    case OPCODE_NOT: InstructionExecutor.not_(timestamp); break;

    case OPCODE_MOV: InstructionExecutor.mov(timestamp); break;

    default: debug::log("[PROCESSOR]: Unhandled opcode: ", opcode, "\n");
            // TODO: INT_ILLEGAL_INSTRUCTION
    }
}

void processor::soft_interruption_ready(const uint64_t int_code)
{
}

processor::Target processor::__InstructionExecutorType__::pop_target()
{
    return Target(CPU);
}
