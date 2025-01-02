#include <SysdarftInstructionExec.h>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

class Exec final : public SysdarftCPUInstructionExecutor {
public:
    bool h_is_break_here()
    {
        return true;
    }

    void h_breakpoint_handler(__uint128_t, const uint8_t opcode, const WidthAndOperandsType & WAOpT)
    {
        show_context();

        for (auto [fst, snd] : instruction_map) {
            if (snd.at(ENTRY_OPCODE) == opcode) {
                log(" => ", fst, " ");
            }
        }

        switch (WAOpT.first) {
        case _8bit_prefix:  log(".8bit "); break;
        case _16bit_prefix: log(".16bit"); break;
        case _32bit_prefix: log(".32bit"); break;
        case _64bit_prefix: log(".64bit"); break;
        default: ; // fall through
        }

        int first = 0;
        for (const auto& op : WAOpT.second) {
            log(" ", op.get_literal(), (first++ == 0 && WAOpT.second.size() > 1 ? "," : ""));
        }

        log("\n\n\n");
    }

    Exec()
    {
        bindBreakpointHandler(this, &Exec::h_breakpoint_handler);
        bindIsBreakHere(this, &Exec::h_is_break_here);

        std::vector<uint8_t> buffer;
        encode_instruction(buffer, "mov .64bit <*1&64($(0), $(0), $(0))>, <$(114514)>");
        encode_instruction(buffer, "mov .64bit <*1&64($(0), $(0), $(8))>, <$(0xFFF)>");
        encode_instruction(buffer, "and .64bit <*1&64($(0), $(0), $(8))>, <*1&64($(0), $(0), $(0))>");
        encode_instruction(buffer, "mov .64bit <%FER0>, <*1&64($(0), $(0), $(8))>");
        encode_instruction(buffer, "or  .32bit <%HER1>, <%HER0>");
        encode_instruction(buffer, "xor .64bit <%FER0>, <%FER0>");
        encode_instruction(buffer, "mov .8bit  <%R0>, <$(0x34)>");
        encode_instruction(buffer, "not .64bit <%FER0>");

        encode_instruction(buffer, "shl .8bit <%R0>, <$(4)>");
        encode_instruction(buffer, "shr .8bit <%R0>, <$(6)>");

        encode_instruction(buffer, "mov .8bit <%R0>, <$(0xF4)>");
        encode_instruction(buffer, "rol .8bit <%R0>, <$(2)>");
        encode_instruction(buffer, "ror .8bit <%R0>, <$(1)>");

        encode_instruction(buffer, "mov .8bit <%R0>, <$(0x8F)>");
        encode_instruction(buffer, "rcl .8bit <%R0>, <$(3)>");
        encode_instruction(buffer, "rcr .8bit <%R0>, <$(3)>");
        encode_instruction(buffer, "rcr .8bit <%R0>, <$(1)>");

        uint64_t off = BIOS_START;
        for (const auto & code : buffer) {
            write_memory(off++, (char*)&code, 1);
        }

        for (int i = 0; i < 20; i++) {
            execute(0);
        }
    }
};

int main()
{
    Exec base;
}
