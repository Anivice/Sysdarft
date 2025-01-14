#include <EncodingDecoding.h>
#include <GlobalEvents.h>
#include <InstructionSet.h>
#include <SysdarftCursesUI.h>
#include <SysdarftInstructionExec.h>

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

    Exec() : SysdarftCPUInstructionExecutor(32 * 1024 * 1024)
    {
        bindBreakpointHandler(this, &Exec::h_breakpoint_handler);
        bindIsBreakHere(this, &Exec::h_is_break_here);

        std::vector < std::vector <uint8_t> > code;
        std::map < std::string, std::pair < uint64_t /* line position */, std::vector < uint64_t > > > defined_line_marker;
        defined_line_marker.emplace("putc", std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
        defined_line_marker.emplace("_start", std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
        defined_line_marker.emplace("_loop", std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
        std::stringstream ascii_code;
        ascii_code << "jmp <%CB>, <_start>                \n";
        ascii_code << "putc:                            \n";
        ascii_code << "  pushall                        \n";
        ascii_code << "  push .16bit <%EXR0>            \n";
        ascii_code << "  mov .16bit <%EXR0>, <$(0)>     \n";
        ascii_code << "  pop .16bit <%EXR1>             \n";
        ascii_code << "  int <$(0x10)>                  \n";
        ascii_code << "  popall                         \n";
        ascii_code << "  ret                            \n";
        ascii_code << "_start:                          \n";
        ascii_code << "  mov .64bit <%SP>, <$(0xFFF)>   \n";
        ascii_code << "  mov .16bit <%EXR0>, <$(0x53)>  \n";
        ascii_code << "  call <%CB>, <putc>               \n";
        ascii_code << "_loop:                           \n";
        ascii_code << "  jmp <%CB>, <_loop>               \n";
        SysdarftCompile(code, ascii_code, 0xC1800, defined_line_marker);

        uint64_t off = BIOS_START;
        for (const auto & linear : code) {
            for (const auto & line : linear) {
                write_memory(off++, (char*)&line, 1);
            }
        }

        for (int i = 0; i < 20; i++) {
            execute(0);
        }
    }
};

int main()
{
    debug::verbose = true;
    Exec base;
    sleep(1);
}
