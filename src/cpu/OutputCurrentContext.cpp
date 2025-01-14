#include <iomanip>
#include <SysdarftInstructionExec.h>
#include <EncodingDecoding.h>
#include <sstream>
#include <algorithm> // for std::min
#include <string>
#include <vector>
#include <cstdint>

/**
 * Generates a string similar to the output of the `xxd` command
 * for the provided vector of bytes.
 *
 * - 16 bytes per line
 * - Left column is an 8-hex-digit address offset
 * - Middle section shows bytes in groups of 2, separated by spaces
 * - After two groups of 8 bytes, print two spaces, then print ASCII
 *   representation of all 16 bytes
 * - If the total size is not a multiple of 16, the last line will be shorter
 *
 * @param data A vector of bytes to dump.
 * @return A string that mimics `xxd` output.
 */
std::string xxd_like_dump(const uint64_t offset, const std::vector<uint8_t>& data)
{
    constexpr std::size_t BYTES_PER_LINE = 16;

    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (std::size_t i = 0; i < data.size(); i += BYTES_PER_LINE)
    {
        ss << std::setw(16) << std::setfill('0') << i + offset << ": ";

        // Print up to 16 bytes in hex, grouped for readability
        // xxd often groups in sets of 2 or 4; here we'll group 2 bytes for a close match

        // 1) Print the hex
        std::size_t lineEnd = (i + BYTES_PER_LINE < data.size())
                                ? (i + BYTES_PER_LINE)
                                : data.size();

        // We'll break them into two sets of 8 (each set is 8 bytes => 4 groups of 2)
        for (std::size_t j = i; j < lineEnd; ++j)
        {
            // Print each byte as 2 hex digits
            ss << std::setw(2) << static_cast<unsigned>(data[j]);

            // Insert a space after each 2 bytes for visual grouping
            // (xxd typically shows "00 11 22 33" with each pair as a "word" grouping)
            // We'll do a space after every 2 bytes => check j vs. i offset
            if ((j - i) % 2 == 1) ss << ' ';

            // For spacing between the 8-byte halves, after 8 bytes, we do an extra space
            if ((j - i + 1) == 8 && (j + 1) < lineEnd) {
                ss << " ";
            }
        }

        // If less than 16 bytes on this line, pad spaces so ASCII aligns
        std::size_t bytesOnThisLine = lineEnd - i;
        if (bytesOnThisLine < BYTES_PER_LINE)
        {
            // Each byte printed as 2 hex chars plus 1 space every 2 bytes => total per line
            // For each missing byte, we skip 2 chars in hex + possible space.
            // A simpler approach: figure out how many 2-byte groups remain.
            const std::size_t groupsMissing = ((BYTES_PER_LINE - bytesOnThisLine) + 1) / 2;
            for (uint64_t gm = 0; gm < groupsMissing; ++gm)
            {
                ss << "     ";
            }
            // Might need an extra space if we didn't reach the 8-byte halfway mark
            if (bytesOnThisLine <= 8) {
                ss << " ";
            }
        }

        ss << "  "; // two spaces before an ASCII section

        // 2) Print ASCII (or '.' for non-printable)
        for (std::size_t j = i; j < lineEnd; ++j)
        {
            unsigned char c = data[j];
            if (c >= 32 && c < 127) {
                ss << c;
            } else {
                ss << '.';
            }
        }

        ss << "\n";
    }

    return ss.str();
}

// Example helper to convert an unsigned value to a hex string
template <typename T>
std::string to_hex_string(T value)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');

    if constexpr (sizeof(T) == 1) {
        oss << std::setw(2);
    } else if constexpr (sizeof(T) == 2) {
        oss << std::setw(4);
    } else if constexpr (sizeof(T) == 4) {
        oss << std::setw(8);
    } else if constexpr (sizeof(T) == 8) {
        oss << std::setw(16);
    }
    // Cast to unsigned type before printing in hexadecimal
    oss << static_cast<uint64_t>(value);
    return oss.str();
}

void SysdarftCPUInstructionExecutor::show_context()
{
    ////////////////////////////////////////////////////////////////////////////////
    // SHOW ALL REGISTERS
    ////////////////////////////////////////////////////////////////////////////////

    log("==============================================================================\n");
    log("Registers:\n");
    // --- 8x 8-bit "RegisterType" registers (R0..R7) ---
    {
        auto R0 = SysdarftRegister::load<RegisterType, 0>();
        auto R1 = SysdarftRegister::load<RegisterType, 1>();
        auto R2 = SysdarftRegister::load<RegisterType, 2>();
        auto R3 = SysdarftRegister::load<RegisterType, 3>();
        auto R4 = SysdarftRegister::load<RegisterType, 4>();
        auto R5 = SysdarftRegister::load<RegisterType, 5>();
        auto R6 = SysdarftRegister::load<RegisterType, 6>();
        auto R7 = SysdarftRegister::load<RegisterType, 7>();

        log("R0 = 0x" + to_hex_string(R0) + " ");
        log("R1 = 0x" + to_hex_string(R1) + " ");
        log("R2 = 0x" + to_hex_string(R2) + " ");
        log("R3 = 0x" + to_hex_string(R3) + " ");
        log("R4 = 0x" + to_hex_string(R4) + " ");
        log("R5 = 0x" + to_hex_string(R5) + " ");
        log("R6 = 0x" + to_hex_string(R6) + " ");
        log("R7 = 0x" + to_hex_string(R7) + "\n");
    }

    // --- 8x 16-bit "ExtendedRegisterType" registers (EXR0..EXR7) ---
    {
        auto EXR0 = SysdarftRegister::load<ExtendedRegisterType, 0>();
        auto EXR1 = SysdarftRegister::load<ExtendedRegisterType, 1>();
        auto EXR2 = SysdarftRegister::load<ExtendedRegisterType, 2>();
        auto EXR3 = SysdarftRegister::load<ExtendedRegisterType, 3>();
        auto EXR4 = SysdarftRegister::load<ExtendedRegisterType, 4>();
        auto EXR5 = SysdarftRegister::load<ExtendedRegisterType, 5>();
        auto EXR6 = SysdarftRegister::load<ExtendedRegisterType, 6>();
        auto EXR7 = SysdarftRegister::load<ExtendedRegisterType, 7>();

        log("EXR0 = 0x" + to_hex_string(EXR0) + " ");
        log("EXR1 = 0x" + to_hex_string(EXR1) + " ");
        log("EXR2 = 0x" + to_hex_string(EXR2) + " ");
        log("EXR3 = 0x" + to_hex_string(EXR3) + "\n");
        log("EXR4 = 0x" + to_hex_string(EXR4) + " ");
        log("EXR5 = 0x" + to_hex_string(EXR5) + " ");
        log("EXR6 = 0x" + to_hex_string(EXR6) + " ");
        log("EXR7 = 0x" + to_hex_string(EXR7) + "\n");
    }

    // --- 8x 32-bit "HalfExtendedRegisterType" registers (HER0..HER7) ---
    {
        auto HER0 = SysdarftRegister::load<HalfExtendedRegisterType, 0>();
        auto HER1 = SysdarftRegister::load<HalfExtendedRegisterType, 1>();
        auto HER2 = SysdarftRegister::load<HalfExtendedRegisterType, 2>();
        auto HER3 = SysdarftRegister::load<HalfExtendedRegisterType, 3>();
        auto HER4 = SysdarftRegister::load<HalfExtendedRegisterType, 4>();
        auto HER5 = SysdarftRegister::load<HalfExtendedRegisterType, 5>();
        auto HER6 = SysdarftRegister::load<HalfExtendedRegisterType, 6>();
        auto HER7 = SysdarftRegister::load<HalfExtendedRegisterType, 7>();

        log("HER0 = 0x" + to_hex_string(HER0) + " ");
        log("HER1 = 0x" + to_hex_string(HER1) + " ");
        log("HER2 = 0x" + to_hex_string(HER2) + " ");
        log("HER3 = 0x" + to_hex_string(HER3) + "\n");
        log("HER4 = 0x" + to_hex_string(HER4) + " ");
        log("HER5 = 0x" + to_hex_string(HER5) + " ");
        log("HER6 = 0x" + to_hex_string(HER6) + " ");
        log("HER7 = 0x" + to_hex_string(HER7) + "\n");
    }

    // --- 16x 64-bit "FullyExtendedRegisterType" registers (FER0..FER15) ---
    {
        auto FER0  = SysdarftRegister::load<FullyExtendedRegisterType, 0>();
        auto FER1  = SysdarftRegister::load<FullyExtendedRegisterType, 1>();
        auto FER2  = SysdarftRegister::load<FullyExtendedRegisterType, 2>();
        auto FER3  = SysdarftRegister::load<FullyExtendedRegisterType, 3>();
        auto FER4  = SysdarftRegister::load<FullyExtendedRegisterType, 4>();
        auto FER5  = SysdarftRegister::load<FullyExtendedRegisterType, 5>();
        auto FER6  = SysdarftRegister::load<FullyExtendedRegisterType, 6>();
        auto FER7  = SysdarftRegister::load<FullyExtendedRegisterType, 7>();
        auto FER8  = SysdarftRegister::load<FullyExtendedRegisterType, 8>();
        auto FER9  = SysdarftRegister::load<FullyExtendedRegisterType, 9>();
        auto FER10 = SysdarftRegister::load<FullyExtendedRegisterType, 10>();
        auto FER11 = SysdarftRegister::load<FullyExtendedRegisterType, 11>();
        auto FER12 = SysdarftRegister::load<FullyExtendedRegisterType, 12>();
        auto FER13 = SysdarftRegister::load<FullyExtendedRegisterType, 13>();
        auto FER14 = SysdarftRegister::load<FullyExtendedRegisterType, 14>();
        auto FER15 = SysdarftRegister::load<FullyExtendedRegisterType, 15>();

        log("FER0  = 0x" + to_hex_string(FER0)  + "\n");
        log("FER1  = 0x" + to_hex_string(FER1)  + "\n");
        log("FER2  = 0x" + to_hex_string(FER2)  + "\n");
        log("FER3  = 0x" + to_hex_string(FER3)  + "\n");
        log("FER4  = 0x" + to_hex_string(FER4)  + "\n");
        log("FER5  = 0x" + to_hex_string(FER5)  + "\n");
        log("FER6  = 0x" + to_hex_string(FER6)  + "\n");
        log("FER7  = 0x" + to_hex_string(FER7)  + "\n");
        log("FER8  = 0x" + to_hex_string(FER8)  + "\n");
        log("FER9  = 0x" + to_hex_string(FER9)  + "\n");
        log("FER10 = 0x" + to_hex_string(FER10) + "\n");
        log("FER11 = 0x" + to_hex_string(FER11) + "\n");
        log("FER12 = 0x" + to_hex_string(FER12) + "\n");
        log("FER13 = 0x" + to_hex_string(FER13) + "\n");
        log("FER14 = 0x" + to_hex_string(FER14) + "\n");
        log("FER15 = 0x" + to_hex_string(FER15) + "\n");
    }

    // --- Flag Register (bitfield) ---
    {
        auto flags = SysdarftRegister::load<FlagRegisterType>();
        log("Flags:\n");
        log("  Carry            = " + std::to_string((int)flags.Carry) + "\n");
        log("  Overflow         = " + std::to_string((int)flags.Overflow) + "\n");
        log("  LargerThan       = " + std::to_string((int)flags.LargerThan) + "\n");
        log("  LessThan         = " + std::to_string((int)flags.LessThan) + "\n");
        log("  Equal            = " + std::to_string((int)flags.Equal) + "\n");
        log("  InterruptionMask = " + std::to_string((int)flags.InterruptionMask) + "\n");
    }

    // --- 64-bit registers: StackBase, StackPointer, CodeBase, InstructionPointer,
    //     DataBase, DataPointer, ExtendedBase, ExtendedPointer,
    //     CurrentProcedureStackPreservationSpace ---
    {
        auto sb  = SysdarftRegister::load<StackBaseType>();
        auto sp  = SysdarftRegister::load<StackPointerType>();
        auto cb  = SysdarftRegister::load<CodeBaseType>();
        auto rip = SysdarftRegister::load<InstructionPointerType>();
        auto db  = SysdarftRegister::load<DataBaseType>();
        auto dp  = SysdarftRegister::load<DataPointerType>();
        auto eb  = SysdarftRegister::load<ExtendedBaseType>();
        auto ep  = SysdarftRegister::load<ExtendedPointerType>();
        auto cps = SysdarftRegister::load<CurrentProcedureStackPreservationSpaceType>();

        log("SB  = 0x" + to_hex_string(sb)  + "\n");
        log("SP  = 0x" + to_hex_string(sp)  + "\n");
        log("CB  = 0x" + to_hex_string(cb)  + "\n");
        log("IP  = 0x" + to_hex_string(rip) + "\n");
        log("DB  = 0x" + to_hex_string(db)  + "\n");
        log("DP  = 0x" + to_hex_string(dp)  + "\n");
        log("EB  = 0x" + to_hex_string(eb)  + "\n");
        log("EP  = 0x" + to_hex_string(ep)  + "\n");
        log("CPS = 0x" + to_hex_string(cps) + "\n");
    }

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW DB:DP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    log("==============================================================================\n");
    log("[DB:DP]:\n");
    auto data_off = SysdarftRegister::load<DataBaseType>() + SysdarftRegister::load<DataPointerType>();
    auto data_len = std::min(TotalMemory - data_off, 128ul);
    std::vector<uint8_t> data_buffer;
    for (uint64_t i = 0; i < data_len; i++) {
        char c;
        SysdarftCPUMemoryAccess::read_memory(data_off + i, &c, 1);
        data_buffer.push_back(c);
    }
    log(xxd_like_dump(data_off, data_buffer), "\n");

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW EB:EP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    log("==============================================================================\n");
    log("[EB:EP]:\n");
    auto ext_off = SysdarftRegister::load<ExtendedBaseType>() + SysdarftRegister::load<ExtendedPointerType>();
    auto ext_len = std::min(TotalMemory - ext_off, 128ul);
    std::vector<uint8_t> ext_buffer;
    for (uint64_t i = 0; i < ext_len; i++) {
        char c;
        SysdarftCPUMemoryAccess::read_memory(ext_off + i, &c, 1);
        ext_buffer.push_back(c);
    }
    log(xxd_like_dump(ext_off, ext_buffer), "\n");

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW SB:SP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    log("==============================================================================\n");
    log("[SB:SP]:\n");
    auto stack_off = SysdarftRegister::load<StackBaseType>() + SysdarftRegister::load<StackPointerType>();
    auto stack_len = std::min(TotalMemory - stack_off, 128ul);
    std::vector<uint8_t> stack_buffer;
    for (uint64_t i = 0; i < stack_len; i++) {
        char c;
        SysdarftCPUMemoryAccess::read_memory(stack_off + i, &c, 1);
        stack_buffer.push_back(c);
    }
    log(xxd_like_dump(stack_off, stack_buffer), "\n");

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW NEXT 8 INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////////
    log("==============================================================================\n");
    log("Following instructions:\n");
    std::vector<std::string> next_8_instructions;
    const uint64_t offset =
          SysdarftRegister::load<CodeBaseType>()
        + SysdarftRegister::load<InstructionPointerType>();
    const uint64_t length = std::min<uint64_t>(256, TotalMemory - offset);
    std::vector<uint8_t> buffer_max256;
    buffer_max256.reserve(length);

    for (uint64_t i = 0; i < length; i++)
    {
        char c;
        SysdarftCPUMemoryAccess::read_memory(offset + i, &c, 1);
        buffer_max256.push_back(static_cast<uint8_t>(c));
    }

    while (!buffer_max256.empty())
    {
        std::stringstream line_num;
        line_num << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
                 << (offset + (length - buffer_max256.size()));

        decode_instruction(next_8_instructions, buffer_max256);

        if (!next_8_instructions.empty()) {
            next_8_instructions.back() =
                line_num.str() + ": " + next_8_instructions.back();
        }
    }

    if (next_8_instructions.size() > 8) {
        next_8_instructions.resize(8);
    }

    for (const auto &instruction : next_8_instructions) {
        log(instruction, "\n");
    }
}
