#include <cpu.h>
#include <array>
#include <fstream>
std::fstream file;

int off = 0;
// add .64bit <%FER1>, <*1($(0), %FER0, $(2)>
std::array<uint8_t, BIOS_SIZE> BIOS = { 0 };

void construct_reg(const uint8_t bcd_width, const uint8_t index)
{
    BIOS[off++] = REGISTER_PREFIX;
    BIOS[off++] = bcd_width;
    BIOS[off++] = index;
}

void construct_64bit_num(const uint64_t val)
{
    BIOS[off++] = ((uint8_t*)&val)[0];
    BIOS[off++] = ((uint8_t*)&val)[1];
    BIOS[off++] = ((uint8_t*)&val)[2];
    BIOS[off++] = ((uint8_t*)&val)[3];
    BIOS[off++] = ((uint8_t*)&val)[4];
    BIOS[off++] = ((uint8_t*)&val)[5];
    BIOS[off++] = ((uint8_t*)&val)[6];
    BIOS[off++] = ((uint8_t*)&val)[7];
}

void construct_constant(const uint64_t val)
{
    BIOS[off++] = CONSTANT_PREFIX;
    BIOS[off++] = 0x64;
    construct_64bit_num(val);
}

int main()
{
    // add .64bit <*2($(0), %FER2, $(4))>, <$(114514)>
    //////////////////////////////////////////////////
    construct_64bit_num(0x01); // add
    BIOS[off++] = 0x64; // .64bit

    // *2($(0), %FER2, $(4))
    BIOS[off++] = MEMORY_PREFIX;
    BIOS[off++] = 0x64;
    construct_constant(0x00);
    construct_reg(0x64, 0x02);
    construct_constant(0x04);
    BIOS[off++] = 2;

    // $(114514)
    construct_constant(114514); // 0x1BF52
    //////////////////////////////////////////////////

    processor CPU;
    CPU.load_bios(BIOS);
    CPU.start_triggering();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CPU.stop_triggering();
}
