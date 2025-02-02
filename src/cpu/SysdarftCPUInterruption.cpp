/* SysdarftCPUInterruption.cpp
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

#include <termios.h>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <GlobalEvents.h>
#include <SysdarftCPUDecoder.h>
#include <SysdarftCursesUI.h>
#include <SysdarftInstructionExec.h>

#ifdef __DEBUG__

void flush_stdin()
{
    if (tcflush(STDIN_FILENO, TCIFLUSH) != 0) {
        log("Error flushing stdin: ", strerror(errno), "\n");
    }
}

#endif

SysdarftCPUInterruption::SysdarftCPUInterruption(const uint64_t memory, const std::string & font_name) :
    DecoderDataAccess(memory, font_name)
{
    // Get the current flags for stdin (file descriptor 0)
    const int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        throw SysdarftCPUInitializeFailed();
    }

    // Set the O_NONBLOCK flag
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw SysdarftCPUInitializeFailed();
    }
}

#ifdef __DEBUG__

void clear_screen()
{
    const char clear[] = { 0x1b, 0x5b, 0x48, 0x1b, 0x5b, 0x32, 0x4a, 0x1b, 0x5b, 0x33, 0x4a };
    write(STDOUT_FILENO, clear, sizeof(clear));
}

std::atomic_bool cleared{false};

#endif

void SysdarftCPUInterruption::do_interruption(const uint64_t code)
{
    if (code > MAX_INTERRUPTION_ENTRY) {
        throw SysdarftBadInterruption("Interruption out of range");
    }

    auto fg = SysdarftRegister::load<FlagRegisterType>();

    auto mask_fg = [&]()
    {
        // mask
        fg.InterruptionMask = 1;
        SysdarftRegister::store<FlagRegisterType>(fg);
    };

    auto do_interruption = [&]()
    {
        // software interruptions, maskable
        const auto location = do_interruption_lookup(code);

        try {
            do_preserve_cpu_state();
        } catch (StackOverflow &) {
            // TL;DR: stackoverflow happened whilst preserving CPU state during an interruption call!
            // Long answer: stackoverflow happened whilst preserving CPU state,
            // this will abort the original interruption procedure,
            // and directly redirect itself to stackoverflow interruption handler.
            // however, this will cause the original reason for interruption to be missing, since, well,
            // stack frame is damaged and cannot be used to preserve any useful information anymore.
            do_stackoverflow_0x07();

            // we are about to abort the cpu instruction execution here since this is a fatal error,
            // and we are not sure who raised this.
            // It can be raised by anyone in any given state, so we abandon the current instruction routine.
            // thus, mask fg will not be executed unless we do it manually again, at here
            mask_fg();

            // Abort current routine
            throw SysdarftCPUSubroutineRequestToAbortTheCurrentInstructionExecutionProcedureDueToError();
        }

        // mask interruption flag
        // this will be done AFTER the preservation of CPU state
        mask_fg();

        // setup jump table
        do_jump_table(location);
    };

    if (code <= 0x1F)
    {
#ifdef __DEBUG__
        if (code < 0x10 && code != 0x09)
        {
            // if (!cleared) {
            //     clear_screen();
            //     cleared = true;
            // }

            SysdarftCursesUI::cleanup();
            log("[CPU INTERRUPT]: \033[31;6;7;1mWarning: Hardware exception thrown with code ", code, "\033[0m\n");
            if (SysdarftCursesUI::get_is_inited())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::start_again();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::cleanup();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::start_again();
            }
        }
        else if (code == 0x09)
        {
            SysdarftCursesUI::cleanup();
            log("[CPU INTERRUPT]: \033[32;6;7;1mSystem shutdown request received!\033[0m\n");
            if (SysdarftCursesUI::get_is_inited())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::start_again();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::cleanup();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                SysdarftCursesUI::start_again();
            }
        }
#else
        if (debug::verbose)
        {
            if (code < 0x10 && code != 0x09)
            {
                SysdarftCursesUI::cleanup();
                std::cerr << "[CPU INTERRUPT]: \033[31;6;7;1mWarning: Hardware exception thrown with code "
                          << code << "\033[0m" << std::endl;
                if (SysdarftCursesUI::get_is_inited())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    SysdarftCursesUI::start_again();
                }
            }
            else if (code == 0x09)
            {
                SysdarftCursesUI::cleanup();
                std::cerr << "[CPU INTERRUPT]: \033[32;6;7;1mSystem shutdown request received!\033[0m" << std::endl;
                if (SysdarftCursesUI::get_is_inited())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    SysdarftCursesUI::start_again();
                }
            }
        }
#endif

        // hardware interruptions, un-maskable
        switch (code) {
        case 0x00: do_interruption_fatal_0x00();            return;
        case 0x03: do_interruption_debug_0x03();            return;
        case 0x07: do_stackoverflow_0x07();                 return;
        case 0x10: do_interruption_tty_0x10();              return;
        case 0x11: do_interruption_set_cur_pos_0x11();      return;
        case 0x12: do_interruption_set_cur_visib_0x12();    return;
        case 0x13: do_interruption_newline_0x13();          return;
        case 0x14: do_interruption_getInput_0x14();         return;
        case 0x15: do_interruption_cur_pos_0x15();          return;
        case 0x16: do_get_system_hardware_info_0x16();      return;
        case 0x17: do_ring_bell_0x17();                     return;
        case 0x18: do_refresh_screen_0x18();                return;
        case 0x19: do_clear_user_input_stream_0x19();       return;
        default:
            // do interruption, but doesn't check interruption mask since
            // this is not maskable interruptions
            do_interruption();
        }
    }

    if (!fg.InterruptionMask) {
        do_interruption();
    }
}

void SysdarftCPUInterruption::do_ext_dev_interruption(const uint64_t code)
{
    if (code > 0x1F && code < MAX_INTERRUPTION_ENTRY)
    {
        if (!External_Int_Req_Vec_Protector.try_lock()) {
            log("External device interruption ignored, number ", code, "\n");
            return; // ignore this interruption, the system is currently masked
            // (the system processing other hardware interruptions, bus not free)
        }

        interruption_requests.emplace_back(code);
        external_device_requested = true;
        External_Int_Req_Vec_Protector.unlock();
    } else {
        throw SysdarftInterruptionOutOfRange("External hardware has invoked a invalid interruption code");
    }
}

bool SysdarftCPUInterruption::try_add_input(const int input_)
{
    // interruption open and seeking input
    if (input_source == 0) {
        input_source = input_;
        return true;
    }

    return false;
}

SysdarftCPUInterruption::InterruptionPointer SysdarftCPUInterruption::do_interruption_lookup(const uint64_t code)
{
    InterruptionPointer pointer { };
    const uint64_t offset = INTERRUPTION_VECTOR + code * sizeof(pointer);
    if (offset >= INTERRUPTION_VEC_ED)
    {
        throw SysdarftInterruptionOutOfRange("No such interruption: " + std::to_string(code));
    }

    SysdarftCPUMemoryAccess::read_memory(offset, (char*)&pointer, sizeof(pointer));
    return pointer;
}

void SysdarftCPUInterruption::do_preserve_cpu_state()
{
    const auto SB = SysdarftRegister::load<StackBaseType>();
    auto SP = SysdarftRegister::load<StackPointerType>();
    DecoderDataAccess::push_memory_to(SB, SP, SysdarftRegister::Registers);
    SysdarftRegister::store<StackPointerType>(SP);
}

void SysdarftCPUInterruption::do_jump_table(const InterruptionPointer & location)
{
    SysdarftRegister::store<CodeBaseType>(location.InterruptionTargetCodeBase);
    SysdarftRegister::store<InstructionPointerType>(location.InterruptionTargetInstructionPointer);
}

void SysdarftCPUInterruption::do_iret()
{
    const auto SB = SysdarftRegister::load<StackBaseType>();
    auto SP = SysdarftRegister::load<StackPointerType>();
    SysdarftRegister::Registers = DecoderDataAccess::pop_memory_from<sysdarft_register_t>(SB, SP);
    // iret doesn't need to reset IM
}

// Hardware Interruptions
void SysdarftCPUInterruption::do_interruption_fatal_0x00()
{
    const auto location = do_interruption_lookup(0x00);
    set_mask(); // we don't preserve the CPU state here so that further damage won't be done
    do_jump_table(location);
}

void SysdarftCPUInterruption::do_interruption_debug_0x03()
{
    Int3DebugInterrupt = true;
    // software interruptions
    const auto location = do_interruption_lookup(0x03);
    do_preserve_cpu_state();
    set_mask();
    do_jump_table(location);
}

void SysdarftCPUInterruption::do_stackoverflow_0x07()
{
    // We do not preserve CPU state here
    // It's impossible since, well, stack frame overflowed and cannot be used
    const auto location = do_interruption_lookup(0x07);
    set_mask();
    do_jump_table(location);
}

void SysdarftCPUInterruption::do_abort_0x05()
{
    const auto location = do_interruption_lookup(0x05);
    do_preserve_cpu_state();
    set_mask();
    do_jump_table(location);
}

void SysdarftCPUInterruption::do_interruption_tty_0x10()
{
    const auto ch = static_cast<char>(SysdarftRegister::load<ExtendedRegisterType, 0>());
    SysdarftCursesUI::teletype(ch);
}

void SysdarftCPUInterruption::do_interruption_set_cur_pos_0x11()
{
    const auto linear = SysdarftRegister::load<ExtendedRegisterType, 0>();
    if (linear > V_WIDTH * V_HEIGHT - 1) {
        throw SysdarftBadInterruption("Teletype linear address out of range");
    }
    const auto y = linear / V_WIDTH, x = linear % V_WIDTH;
    SysdarftCursesUI::set_cursor(x, y);
}

void SysdarftCPUInterruption::do_interruption_set_cur_visib_0x12()
{
    const auto vsb = static_cast<bool>(SysdarftRegister::load<ExtendedRegisterType, 0>());
    SysdarftCursesUI::set_cursor_visibility(vsb);
}

void SysdarftCPUInterruption::do_interruption_newline_0x13()
{
    SysdarftCursesUI::newline();
}

void SysdarftCPUInterruption::do_interruption_getInput_0x14()
{
    while (!SystemHalted)
        // abort if external halt or device interruption triggered
    {
        if (KeyboardIntAbort || debugger_pause_blocked_int_0x14 || CtrlZShutdownRequested)
        {
            debugger_pause_blocked_int_0x14 = false;
            // revert ip to int <$(0x14)>
            SysdarftRegister::store<InstructionPointerType>(ip_before_pop);
            return;
        }

        if (input_source != 0) {
            SysdarftRegister::store<ExtendedRegisterType, 0>(input_source);
            input_source = 0;
            return;
        }

        if (const auto Key = SysdarftCursesUI::get_input(); Key != -1)
        {
            SysdarftRegister::store<ExtendedRegisterType, 0>(Key);
            return;
        }

        if (external_device_requested)
        {
            // revert IP, so when it comes back it's still requesting input
            auto IP = SysdarftRegister::load<InstructionPointerType>();
            IP -= current_routine_pop_len;
            SysdarftRegister::store<InstructionPointerType>(IP);

            auto FG = SysdarftRegister::load<FlagRegisterType>();
            FG.InterruptionMask = 0;
            SysdarftRegister::store<FlagRegisterType>(FG);

            return; // abort when an external device called
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void SysdarftCPUInterruption::do_interruption_cur_pos_0x15()
{
    const uint16_t linear = cursor_y * V_WIDTH + cursor_x;
    SysdarftRegister::store<ExtendedRegisterType, 0>(linear);
}

void SysdarftCPUInterruption::do_get_system_hardware_info_0x16()
{
    SysdarftRegister::store<FullyExtendedRegisterType, 0>(TotalMemory);
}

void SysdarftCPUInterruption::do_ring_bell_0x17()
{
    SysdarftCursesUI::ringbell();
}

void SysdarftCPUInterruption::do_refresh_screen_0x18()
{
    SysdarftCursesUI::render_screen();
}

void SysdarftCPUInterruption::do_clear_user_input_stream_0x19()
{
    SysdarftCursesUI::flush_input_buffer();
}
