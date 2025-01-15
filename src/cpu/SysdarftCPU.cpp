#include <SysdarftCPU.h>
#include <SysdarftDisks.h>

SysdarftCPU::SysdarftCPU(const uint64_t memory,
    const std::vector < uint8_t > & bios,
    const std::string & hdd,
    const std::string & fda,
    const std::string & fdb)
        : SysdarftCPUInstructionExecutor(memory)
{
    // load BIOS to memory
    uint64_t off = BIOS_START;
    for (const auto & c : bios) {
        write_memory(off++, (char*)&c, 1);
    }

    // hard disk
    if (!hdd.empty()) {
        device_list.emplace_back(std::make_unique<SysdarftHardDisk>(hdd));
    }

    // floppy disk a
    if (!fda.empty()) {
        device_list.emplace_back(std::make_unique<SysdarftFloppyDiskA>(fda));
    }

    // floppy disk b (not bootable)
    if (!fdb.empty()) {
        device_list.emplace_back(std::make_unique<SysdarftFloppyDiskB>(fdb));
    }
}

void SysdarftCPU::Boot()
{
    SystemHalted = false;
    do_abort_int = false;
    hd_int_flag = false;
    timestamp = 0;

    SysdarftCursesUI::initialize();

    while (!SystemHalted)
    {
        // capture and control area
        if (do_abort_int)
        {
            do_abort_int = false;
            do_abort_0x05();
        }

        try {
            SysdarftCPUInstructionExecutor::execute(timestamp++);
        } catch (std::exception & e) {
            std::cerr << "Unexpected error detected: " << e.what() << std::endl;
        }
    }

    SysdarftCursesUI::cleanup();
}
