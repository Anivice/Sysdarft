#include <GlobalEvents.h>
#include <SysdarftCPUDecoder.h>
#include <SysdarftCursesUI.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

int my_getch()
{
    termios oldt{}, newt{};

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Disable canonical mode and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    const int ch = getchar();

    // Restore original settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

SysdarftCPUInterruption::SysdarftCPUInterruption(const uint64_t memory) :
    DecoderDataAccess(memory)
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

void SysdarftCPUInterruption::do_interruption(const uint64_t code)
{
    if (code <= 0x1F)
    {
        // hardware interruptions, unmaskable
        switch (code) {
        case 0x00: do_interruption_fatal_0x00();            return;
        case 0x03: do_interruption_debug_0x03();            return;
        case 0x10: do_interruption_tty_0x10();              return;
        case 0x11: do_interruption_set_cur_pos_0x11();      return;
        case 0x12: do_interruption_set_cur_visib_0x12();    return;
        case 0x13: do_interruption_newline_0x13();          return;
        case 0x14: do_interruption_getinput_0x14();         return;
        case 0x15: do_interruption_cur_pos_0x15();          return;
        default: log("Hardware interruption ", code, " not implemented\n"); return;
        }
    }

    if (auto fg = SysdarftRegister::load<FlagRegisterType>();
        !fg.InterruptionMask)
    {
        // software interruptions, maskable
        const auto location = do_interruption_lookup(code);
        do_preserve_cpu_state();

        // mask
        fg.InterruptionMask = 1;
        SysdarftRegister::store<FlagRegisterType>(fg);

        // setup jump table
        do_jump_table(location);
    }
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
    // SysdarftRegister::store<StackPointerType>(SP);
}

// Hardware Interruptions
void SysdarftCPUInterruption::do_interruption_fatal_0x00()
{
    log("Unrecoverable Fatal Error!");
    throw SysdarftCPUFatal();
}

void SysdarftCPUInterruption::do_interruption_debug_0x03()
{
    hd_int_flag = true;
    // software interruptions
    const auto location = do_interruption_lookup(0x03);
    do_preserve_cpu_state();
    do_jump_table(location);
}

void SysdarftCPUInterruption::do_abort_0x05()
{
    const auto location = do_interruption_lookup(0x05);
    do_preserve_cpu_state();
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
    const auto x = linear / V_WIDTH, y = linear % V_WIDTH;
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

void SysdarftCPUInterruption::do_interruption_getinput_0x14()
{
    char ch;
    while (!SystemHalted) // abort if external halt is triggered
    {
        if (const ssize_t n = read(STDIN_FILENO, &ch, 1); n > 0)
        {
            SysdarftRegister::store<ExtendedRegisterType, 0>(ch);
            return;
        } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            throw SysdarftCPUFatal();
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
