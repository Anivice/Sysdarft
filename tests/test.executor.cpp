#include <SysdarftInstructionExec.h>
#include <EncodingDecoding.h>

class Exec : public SysdarftCPUInstructionExecutor {
public:

    Exec()
    {
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
