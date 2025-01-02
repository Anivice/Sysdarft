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
        log("\n\n\n");

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
        log("\n");

        show_context();
    }

    Exec()
    {
        bindBreakpointHandler(this, &Exec::h_breakpoint_handler);
        bindIsBreakHere(this, &Exec::h_is_break_here);

        std::vector<uint8_t> buffer;
        encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
        encode_instruction(buffer, "add .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
        encode_instruction(buffer, "mov .64bit <%FER0>, <*2&64($(255), %FER14, $(4))>");
        encode_instruction(buffer, "add .8bit <%R2>, <$(0xFF)>");
        encode_instruction(buffer, "add .8bit <%R3>, <$(0xA0)>");
        encode_instruction(buffer, "add .8bit <%R0>, <$(0x02)>");
        encode_instruction(buffer, "add .8bit <%R1>, <$(0x30)>");
        encode_instruction(buffer, "add .8bit <%R0>, <%R2>");

        uint64_t off = BIOS_START;
        for (const auto & code : buffer) {
            write_memory(off++, (char*)&code, 1);
        }

        for (int i = 0; i < 8; i++) {
            execute(0);
        }
    }
};

int main()
{
    debug::verbose = true;
    Exec base;
}
