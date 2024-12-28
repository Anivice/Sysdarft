#include <cpu.h>
#include <debug.h>

void replace_all(std::string & input, const std::string & target, const std::string & replacement)
{
    if (target.empty()) return; // Avoid infinite loop if target is empty

    size_t pos = 0;
    while ((pos = input.find(target, pos)) != std::string::npos) {
        input.replace(pos, target.length(), replacement);
        pos += replacement.length(); // Move past the replacement to avoid infinite loop
    }
};

template < typename RegType, unsigned SIZE = sizeof(RegType) * 2 >
void generate_reg_info(std::stringstream & ss,
    const std::string & prefix,
    const std::string & suffix,
    const std::string & name,
    const RegType & val)
{
    ss  << _PURPLE_ << name << _REGULAR_ << prefix << _BLUE_
        << std::hex << std::uppercase
        << std::setw(SIZE) << std::setfill('0')
        << static_cast<uint64_t>(val)
        << suffix << _REGULAR_;
}

void generate_config_info(std::stringstream & ss,
    const std::string & name,
    const sysdarft_register_t::SegmentationConfigurationRegister & Configuration,
    const int spacing = 2)
{
    ss  << _PURPLE_ "<" << name << ":BA>:" << std::string(spacing, ' ') << _REGULAR_ "[" _BLUE_
        << std::hex << std::uppercase << std::setw(16) << std::setfill('0')
        << Configuration.BaseAddress << _REGULAR_ "] "
        << _PURPLE_ "<" << name << ":AL>:" << std::string(spacing, ' ') << _REGULAR_ "[" _BLUE_
        << std::hex << std::uppercase << std::setw(16) << std::setfill('0')
        << Configuration.AddressLimit << _REGULAR_ "]" << std::endl;
}

void processor::output_debug_info() {
    std::stringstream ss;
    uint64_t ip;

    // Register view
    ss  << _YELLOW_ _BOLD_ << std::string(94, '=') << ">>> REGISTERS <<<"
        << std::string(79, '=')  << _REGULAR_ << std::endl;
    {
        std::lock_guard lock(RegisterAccessMutex);
        generate_reg_info(ss, ":   ", "               ", "R0", Registers.Register0);
        generate_reg_info(ss, ":   ", "               ", "R1", Registers.Register1);
        generate_reg_info(ss, ":    ", "               ", "R2", Registers.Register2);
        generate_reg_info(ss, ":    ", "               ", "R3", Registers.Register3);
        generate_reg_info(ss, ":    ", "               ", "R4", Registers.Register4);
        generate_reg_info(ss, ":    ", "               ", "R5", Registers.Register5);
        generate_reg_info(ss, ":    ", "               ", "R6", Registers.Register6);
        generate_reg_info(ss, ":    ", "               ", "R7", Registers.Register7);

        ss << std::endl;

        generate_reg_info(ss, ": ", "             ", "EXR0", Registers.ExtendedRegister0);
        generate_reg_info(ss, ": ", "             ", "EXR1", Registers.ExtendedRegister1);
        generate_reg_info(ss, ":  ", "             ", "EXR2", Registers.ExtendedRegister2);
        generate_reg_info(ss, ":  ", "             ", "EXR3", Registers.ExtendedRegister3);
        generate_reg_info(ss, ":  ", "             ", "EXR4", Registers.ExtendedRegister4);
        generate_reg_info(ss, ":  ", "             ", "EXR5", Registers.ExtendedRegister5);
        generate_reg_info(ss, ":  ", "             ", "EXR6", Registers.ExtendedRegister6);
        generate_reg_info(ss, ":  ", "             ", "EXR7", Registers.ExtendedRegister7);

        ss << std::endl;

        generate_reg_info(ss, ": ", "         ", "HER0", Registers.HalfExtendedRegister0);
        generate_reg_info(ss, ": ", "         ", "HER1", Registers.HalfExtendedRegister1);
        generate_reg_info(ss, ":  ", "         ", "HER2", Registers.HalfExtendedRegister2);
        generate_reg_info(ss, ":  ", "         ", "HER3", Registers.HalfExtendedRegister3);
        generate_reg_info(ss, ":  ", "         ", "HER4", Registers.HalfExtendedRegister4);
        generate_reg_info(ss, ":  ", "         ", "HER5", Registers.HalfExtendedRegister5);
        generate_reg_info(ss, ":  ", "         ", "HER6", Registers.HalfExtendedRegister6);
        generate_reg_info(ss, ":  ", "         ", "HER7", Registers.HalfExtendedRegister7);

        ss << std::endl;

        generate_reg_info(ss, ": ", " ", "FER0", Registers.FullyExtendedRegister0);
        generate_reg_info(ss, ": ", " ", "FER1", Registers.FullyExtendedRegister1);
        generate_reg_info(ss, ":  ", " ", "FER2", Registers.FullyExtendedRegister2);
        generate_reg_info(ss, ":  ", " ", "FER3", Registers.FullyExtendedRegister3);
        generate_reg_info(ss, ":  ", " ", "FER4", Registers.FullyExtendedRegister4);
        generate_reg_info(ss, ":  ", " ", "FER5", Registers.FullyExtendedRegister5);
        generate_reg_info(ss, ":  ", " ", "FER6", Registers.FullyExtendedRegister6);
        generate_reg_info(ss, ":  ", " ", "FER7", Registers.FullyExtendedRegister7);

        ss << std::endl;

        generate_reg_info(ss, ": ", " ", "FER8", Registers.FullyExtendedRegister8);
        generate_reg_info(ss, ": ", " ", "FER9", Registers.FullyExtendedRegister9);
        generate_reg_info(ss, ": ", " ", "FER10", Registers.FullyExtendedRegister10);
        generate_reg_info(ss, ": ", " ", "FER11", Registers.FullyExtendedRegister11);
        generate_reg_info(ss, ": ", " ", "FER12", Registers.FullyExtendedRegister12);
        generate_reg_info(ss, ": ", " ", "FER13", Registers.FullyExtendedRegister13);
        generate_reg_info(ss, ": ", " ", "FER14", Registers.FullyExtendedRegister14);
        generate_reg_info(ss, ": ", " ", "FER15", Registers.FullyExtendedRegister15);
        ip = Registers.InstructionPointer;

        ss << std::endl;

        ss  << _PURPLE_ "<FR:CF>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.Carry & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:OF>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.Overflow & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:LE>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.LessThan & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:GR>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.LargerThan & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:EQ>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.Equal & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:IM>: " _REGULAR_ "[" << _BLUE_ << (Registers.FlagRegister.InterruptionMask & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<FR:CP>: " _REGULAR_ "[" << _BLUE_
            << std::setw(2) << std::setfill('0') << std::uppercase
            << (Registers.FlagRegister.CurrentPrivilegeLevel & 0xFF) << _REGULAR_ "]"
            << std::endl;

        ss  << _PURPLE_ "<CR0:PM>: " _REGULAR_ "["
            << _BLUE_ << (Registers.ControlRegister0.ProtectedMode & 0x01) << _REGULAR_ "] "
            << _PURPLE_ "<CR0:PG>: " _REGULAR_ "["
            << _BLUE_ << (Registers.ControlRegister0.Paging & 0x01) << _REGULAR_ "] "
            << std::endl;

        generate_reg_info(ss, ":  ", " ", "SP", Registers.StackPointer);
        ss << " ";
        generate_config_info(ss, "SC", Registers.StackConfiguration);

        generate_reg_info(ss, ":  ", " ", "IP", Registers.InstructionPointer);
        ss << " ";
        generate_config_info(ss, "CC", Registers.CodeConfiguration);

        generate_reg_info(ss, ":  ", " ", "DP", Registers.DataPointer);
        ss << " ";
        generate_config_info(ss, "DC", Registers.DataConfiguration);

        generate_reg_info(ss, ": ", " ", "ESP", Registers.ExtendedSegmentPointer);
        ss << " ";
        generate_config_info(ss, "ESC", Registers.ExtendedSegmentConfiguration, 1);

        ss  << _PURPLE_ "XMM0: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM1: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM2: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM3: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM4: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM5: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM6: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_ << std::endl
            << _PURPLE_ "XMM7: " _REGULAR_ _BLUE_ << Registers.XMM0 << _REGULAR_;
    }

    // Instruction decoder
    ss  << "\n\n" _YELLOW_ _BOLD_ << std::string(94, '=') << ">>> NEXT 8 INSTRUCTIONS <<<"
        << std::string(69, '=') << _REGULAR_ << std::endl;

    std::string decoded =
        rlmode_decode_instruction_within_range(ip, +256);
    replace_all(decoded, "\n", "\n" + std::string(16, ' '));

    ss << _GREEN_ _BOLD_ " =============> " _REGULAR_;

    int line = 1;
    for (const auto & ch : decoded)
    {
        ss << ch;
        if (ch == '\n') {
            line ++;
        }
        if (line == 8) {
            break;
        }
    }

    debug::log(ss.str().c_str(), "\n");
}
