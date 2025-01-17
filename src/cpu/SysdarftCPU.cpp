#include <SysdarftCPU.h>
#include <SysdarftDisks.h>
#include <RealTimeClock.h>

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
        add_device<SysdarftHardDisk>(hdd);
    }

    // floppy disk a
    if (!fda.empty()) {
        add_device<SysdarftFloppyDiskA>(fda);
    }

    // floppy disk b (not bootable)
    if (!fdb.empty()) {
        add_device<SysdarftFloppyDiskB>(fda);
    }

    // RTC
    add_device<SysdarftRealTimeClock>(*this);

    // reset timestamp
    timestamp = 0;
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
            // the reason why 0x05 is raised using flags is that
            // we don't want the program to be halting inside a
            // signal capturing state where thread safety is harder to regulate.
            // also, if we interrupt whist protector is locked, it will cause a deadlock
            do_abort_int = false;
            do_abort_0x05();
        }

        SysdarftCPUInterruption::protector.lock();

        for (const auto & i : interruption_requests) {
            do_interruption(i);
            external_device_requested = false;
        }

        interruption_requests.clear();
        SysdarftCPUInstructionExecutor::protector.unlock();

        try {
            SysdarftCPUInstructionExecutor::execute(timestamp++);
        } catch (std::exception & e) {
            std::cerr << "Unexpected error detected: " << e.what() << std::endl;
        }
    }

    SysdarftCursesUI::cleanup();
}
