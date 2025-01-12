#include <SysdarftCPUDecoder.h>
#include <GlobalEvents.h>
#include <SysdarftCursesUI.h>

void SysdarftCPUInterruption::do_interruption(const uint64_t code)
{
    if (code <= 0x1F)
    {
        // hardware interruptions
        switch (code) {
        case 0x00: do_interruption_fatal_0x00(); return;
        case 0x03: do_interruption_debug_0x03(); return;
        case 0x10: do_interruption_tty_0x10(); return;
        case 0x11: do_interruption_set_cur_pos_0x11(); return;
        case 0x12: do_interruption_set_cur_visib_0x12(); return;
        default: log("Interruption ", code, " not implemented\n"); return;
        }
    }

    // software interruptions
    const auto location = do_interruption_lookup(code);
    do_preserve_cpu_state();
    do_jump_table(location);
}

SysdarftCPUInterruption::InterruptionPointer SysdarftCPUInterruption::do_interruption_lookup(uint64_t code)
{
    InterruptionPointer pointer{};
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

class SysdarftCPUFatal final : public SysdarftBaseError {
public:
    SysdarftCPUFatal() : SysdarftBaseError("CPU is met with a unrecoverable fatal error") { }
};

class SysdarftBadInterruption final : public SysdarftBaseError {
public:
    explicit SysdarftBadInterruption(const std::string & msg) : SysdarftBaseError("Bad interruption: " + msg) { }
};

// Hardware Interruptions
void SysdarftCPUInterruption::do_interruption_fatal_0x00()
{
    log("Unrecoverable Fatal Error!");
    throw SysdarftCPUFatal();
}

void SysdarftCPUInterruption::do_interruption_debug_0x03()
{
    hd_int_flag = true;
}

void SysdarftCPUInterruption::do_interruption_tty_0x10()
{
    const auto linear = SysdarftRegister::load<ExtendedRegisterType, 0>();
    if (linear > V_WIDTH * V_HEIGHT - 1) {
        throw SysdarftBadInterruption("Teletype linear address out of range");
    }
    const auto ch = static_cast<char>(SysdarftRegister::load<ExtendedRegisterType, 1>());
    const auto x = linear / V_WIDTH, y = linear % V_WIDTH;

    std::string msg;
    msg += ch;

    g_ui_teletype(x, y, msg);
}

void SysdarftCPUInterruption::do_interruption_set_cur_pos_0x11()
{
    const auto linear = SysdarftRegister::load<ExtendedRegisterType, 0>();
    if (linear > V_WIDTH * V_HEIGHT - 1) {
        throw SysdarftBadInterruption("Teletype linear address out of range");
    }
    const auto x = linear / V_WIDTH, y = linear % V_WIDTH;
    g_ui_set_cursor(x, y);
}

void SysdarftCPUInterruption::do_interruption_set_cur_visib_0x12()
{
    const auto vsb = static_cast<bool>(SysdarftRegister::load<ExtendedRegisterType, 0>());
    g_ui_set_cur_vsb(vsb);
}
