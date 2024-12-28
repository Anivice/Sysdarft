#include <cpu.h>
#include <array>

int off = 0;
std::array<uint8_t, BIOS_SIZE> BIOS = { 0 };

int main()
{
    // debug::verbose = true;
    std::vector<uint8_t> buffer;
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(6))>, <$(0xFFF)>");
    encode_instruction(buffer, "and .64bit <*2&64($(255), %FER14, $(4))>, <*2&64($(255), %FER14, $(6))>");
    encode_instruction(buffer, "mov .64bit <%FER0>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "or .32bit <%HER1>, <%HER0>");
    encode_instruction(buffer, "xor .64bit <%FER0>, <%FER0>");
    encode_instruction(buffer, "mov .8bit <%R0>, <$(0x34)>");
    encode_instruction(buffer, "not .64bit <%FER0>");

    encode_instruction(buffer, "shl .8bit <%R0>, <$(4)>");
    encode_instruction(buffer, "shr .8bit <%R0>, <$(6)>");

    encode_instruction(buffer, "mov .8bit <%R0>, <$(0xF4)>");
    encode_instruction(buffer, "rol .8bit <%R0>, <$(2)>");
    encode_instruction(buffer, "ror .8bit <%R0>, <$(1)>");

    encode_instruction(buffer, "mov .8bit <%R0>, <$(0x8F)>");
    encode_instruction(buffer, "rcl .8bit <%R0>, <$(1)>");
    encode_instruction(buffer, "rcr .8bit <%R0>, <$(1)>");

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
