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
    encode_instruction(buffer, "mov .64bit <%FER0>, <*2&64($(255), %FER14, $(6))>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "xchg .64bit <%FER0>, <%FER1>");
    encode_instruction(buffer, "mov .64bit <%SP>, <$(0xFFFF)>");
    encode_instruction(buffer, "push .64bit <%FER0>");
    encode_instruction(buffer, "pop .64bit <%FER2>");
    encode_instruction(buffer, "pushall");
    encode_instruction(buffer, "div .64bit <%FER1>");
    encode_instruction(buffer, "popall");
    encode_instruction(buffer, "enter .64bit <$(0xFF)>");
    encode_instruction(buffer, "leave");
    encode_instruction(buffer, "mov .64bit <%FER0>, <$(0x00)>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <$(0xC1800)>");
    encode_instruction(buffer, "mov .64bit <%FER2>, <$(0xFFF)>");
    encode_instruction(buffer, "movs");

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
