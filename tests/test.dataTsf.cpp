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
        encode_instruction(buffer, "mov .64bit <%FER1>, <$(0xFFF)>");
        encode_instruction(buffer, "xchg .64bit <*1&64($(0), $(0), $(0))>, <%FER1>");
        encode_instruction(buffer, "mov .64bit <%SP>, <$(0xFFFF)>");
        encode_instruction(buffer, "push .64bit <%FER1>");
        encode_instruction(buffer, "pop .64bit <%FER2>");
        encode_instruction(buffer, "pushall");
        encode_instruction(buffer, "div .64bit <%FER1>");
        encode_instruction(buffer, "popall");
        encode_instruction(buffer, "enter .64bit <$(0xFF)>");
        encode_instruction(buffer, "leave");
        encode_instruction(buffer, "mov .64bit <%DP>, <$(0x00)>");
        encode_instruction(buffer, "mov .64bit <%EP>, <$(0xC1800)>");
        encode_instruction(buffer, "mov .64bit <%FER0>, <$(0xFFF)>");
        encode_instruction(buffer, "movs");

        uint64_t off = BIOS_START;
        for (const auto & code : buffer) {
            write_memory(off++, (char*)&code, 1);
        }

        for (int i = 0; i < 16; i++) {
            execute(0);
        }
    }
};

int main()
{
    Exec base;
}
