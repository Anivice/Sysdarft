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

SysdarftCPU::SysdarftCPU(const uint64_t memory, const std::string & font_name,
    const std::vector < uint8_t > & bios,
    const std::string & hdd,
    const std::string & fda,
    const std::string & fdb)
        : SysdarftCPUInstructionExecutor(memory, font_name)
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
        add_device<SysdarftBlockDevices>(hdd);
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

uint64_t SysdarftCPU::Boot(const bool headless, const bool with_gui)
{
    SystemHalted = false;
    KeyboardIntAbort = false;
    Int3DebugInterrupt = false;
    timestamp = 0;

    if (!headless) {
        SysdarftCursesUI::initialize();
    }

    if (with_gui) {
        SysdarftCursesUI::launch_gui();
    }

    auto invoke_shutdown = [&]()
    {
        have_I_invoked_shutdown = true;
        CtrlZShutdownRequested = false;
        do_interruption(INT_SYSTEM_SHUTDOWN);
    };

    auto cleanup_invoke_shutdown = [&]()
    {
        have_I_invoked_shutdown = false;
        CtrlZShutdownRequested = false;
    };

    cleanup_invoke_shutdown();

    while (!SystemHalted)
    {
        try {
            // capture and control area
            if (KeyboardIntAbort)
            {
                // the reason why 0x05 is raised using flags is that
                // we don't want the program to be halting inside a
                // signal capturing state where thread safety is harder to regulate.
                // also, if we interrupt whist protector is locked, it will cause a deadlock
                KeyboardIntAbort = false;
                do_abort_0x05();
            }

            // external device
            SysdarftCPUInterruption::External_Int_Req_Vec_Protector.lock();

            for (const auto & i : interruption_requests) {
                do_interruption(i);
                external_device_requested = false;
            }
        } catch (SysdarftCPUSubroutineRequestToAbortTheCurrentInstructionExecutionProcedureDueToError&) {
            try {
                do_stackoverflow_0x07();
            } catch (...) {
                std::cerr << "Critical error detected in Sysdarft!" << std::endl;
                show_context();
                return EXIT_FAILURE;
            }
        }

        interruption_requests.clear();
        SysdarftCPUInstructionExecutor::External_Int_Req_Vec_Protector.unlock();

        // shutdown request
        if (CtrlZShutdownRequested)
        {
            if (!have_I_invoked_shutdown) {
                invoke_shutdown();
            }

            // else if (have_I_invoked_shutdown && SysdarftRegister::load<FlagRegisterType>().InterruptionMask) {
            //     // ignore
            // }

            else if (have_I_invoked_shutdown // I have invoked before
                && !SysdarftRegister::load<FlagRegisterType>().InterruptionMask // but not in the interrupt procedure,
                // meaning: shutdown requested, handled, and refused, so we request again
                )
            {
                cleanup_invoke_shutdown();
                invoke_shutdown();
            }

            // else {
                // logically impossible
            // }
        }

        try {
            SysdarftCPUInstructionExecutor::execute(timestamp++);
        } catch (std::exception & e) {
            std::cerr << "Unexpected error detected: " << e.what() << std::endl;
        }
    }

    SysdarftCursesUI::cleanup();

    return SysdarftRegister::load<FullyExtendedRegisterType, 0>();
}
