/* SysdarftCPU.cpp
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    constexpr uint64_t off = BIOS_START;
    uint64_t size = bios.size();
    if (bios.size() > BIOS_SIZE) {
        size = BIOS_SIZE;
    }
    write_memory(off, (char*)bios.data(), size);


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
