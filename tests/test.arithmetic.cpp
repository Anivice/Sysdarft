#include <SysdarftInstructionExec.h>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

class Exec final : public SysdarftCPUInstructionExecutor {
public:
    bool h_is_break_here(__uint128_t)
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

    Exec() : SysdarftCPUInstructionExecutor(32 * 1024 * 1024)
    {
        bindBreakpointHandler(this, &Exec::h_breakpoint_handler);
        bindIsBreakHere(this, &Exec::h_is_break_here);

        std::vector<uint8_t> buffer;
        encode_instruction(buffer, "mov .8bit <%R0>, <$(0xFF)>");
        encode_instruction(buffer, "mov .8bit <%R1>, <$(0xA0)>");

        encode_instruction(buffer, "mov .8bit <%R2>, <$(0xAC)>");
        encode_instruction(buffer, "mov .8bit <%R3>, <$(0xD3)>");

        encode_instruction(buffer, "add .8bit <%R0>, <%R2>");
        encode_instruction(buffer, "adc .8bit <%R1>, <%R3>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .8bit <%R0>, <$(0x00)>");
        encode_instruction(buffer, "mov .8bit <%R1>, <$(0x30)>");

        encode_instruction(buffer, "mov .8bit <%R2>, <$(0x7C)>");
        encode_instruction(buffer, "mov .8bit <%R3>, <$(0x2F)>");

        encode_instruction(buffer, "sub .8bit <%R0>, <%R2>");
        encode_instruction(buffer, "sbb .8bit <%R1>, <%R3>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .64bit <%FER0>, <$(-1)>");
        encode_instruction(buffer, "mov .64bit <%FER1>, <$(-512)>");

        encode_instruction(buffer, "imul .64bit <%FER1>");
        encode_instruction(buffer, "mul  .64bit <$(2)>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .64bit <%FER0>, <$(-1024)>");
        encode_instruction(buffer, "mov .64bit <%FER1>, <$(-13)>");

        encode_instruction(buffer, "idiv .64bit <%FER1>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .64bit <%FER0>, <$(672)>");
        encode_instruction(buffer, "mov .64bit <%FER1>, <$(7)>");

        encode_instruction(buffer, "div .64bit <%FER1>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .64bit <%FER0>, <$(1)>");
        encode_instruction(buffer, "neg .64bit <%FER0>");

        ///////////////////////////////////////////////////////////////

        encode_instruction(buffer, "mov .8bit <*1&8($(0), $(0), $(0))>, <$(0xFFCAE)>");
        encode_instruction(buffer, "mov .8bit <*1&8($(0), $(0), $(2))>, <$(0xFF3321)>");

        encode_instruction(buffer, "cmp .8bit <*1&8($(0), $(0), $(0))>, <*1&8($(0), $(0), $(2))>");

        uint64_t off = BIOS_START;
        for (const auto & code : buffer) {
            write_memory(off++, (char*)&code, 1);
        }

        for (int i = 0; i < 32; i++) {
            execute(0);
        }
    }
};

int main()
{
    Exec base;
}
