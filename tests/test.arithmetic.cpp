#include <cpu.h>
#include <array>
#include <fstream>
std::fstream file;

int off = 0;
std::array<uint8_t, BIOS_SIZE> BIOS = { 0 };

int main()
{
    // debug::verbose = true;
    std::vector<uint8_t> buffer;
    encode_instruction(buffer, "add .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "add .64bit <%FER14>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "add .8bit <%R2>, <$(0xFF)>");
    encode_instruction(buffer, "add .8bit <%R3>, <$(0xA0)>");

    encode_instruction(buffer, "add .8bit <%R0>, <$(0x02)>");
    encode_instruction(buffer, "add .8bit <%R1>, <$(0x30)>");

    encode_instruction(buffer, "add .8bit <%R0>, <%R2>");
    encode_instruction(buffer, "adc .8bit <%R1>, <%R3>");

    encode_instruction(buffer, "sub .16bit <%EXR0>, <$(0xFFFF)>");

    encode_instruction(buffer, "mov .16bit <%EXR0>, <$(-32)>");
    encode_instruction(buffer, "imul .16bit <$(-2)>");
    encode_instruction(buffer, "mov .32bit <%HER0>, <$(65536)>");
    encode_instruction(buffer, "mov .32bit <%HER2>, <$(0x02)>");
    encode_instruction(buffer, "mul .32bit <%HER2>");

    encode_instruction(buffer, "mov .64bit <%FER0>, <$(-65536)>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <$(-2)>");
    encode_instruction(buffer, "idiv .64bit <%FER1>");
    encode_instruction(buffer, "div .64bit <$(3)>");

    encode_instruction(buffer, "neg .64bit <%FER0>");
    encode_instruction(buffer, "cmp .16bit <%EXR0>, <%EXR1>");

    encode_instruction(buffer, "nop");

    for (const auto& code : buffer) {
        BIOS[off++] = code;
    }

    processor CPU;
    CPU.load_bios(BIOS);
    CPU.break_here();
    CPU.start_triggering();
    CPU.breakpoint_handler = [](__uint128_t,
        SysdarftRegister & Registers,
        std::vector < std::array < unsigned char, PAGE_SIZE > > &)
    {

    };

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CPU.stop_triggering();
}
