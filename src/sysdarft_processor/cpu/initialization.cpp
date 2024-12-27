#include <cpu.h>

void processor::load_bios(std::array<uint8_t, BIOS_SIZE> const &bios)
{
    write_memory(BIOS_START, (char*)bios.data(), BIOS_SIZE);
}
