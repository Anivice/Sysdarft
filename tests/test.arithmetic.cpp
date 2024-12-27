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
    encode_instruction(buffer, "add .64bit <*2&64($(0), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "add .64bit <%FER14>, <*2&64($(0), %FER14, $(4))>");
    for (const auto& code : buffer) {
        BIOS[off++] = code;
    }

    processor CPU;
    CPU.load_bios(BIOS);
    CPU.start_triggering();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CPU.stop_triggering();
}
