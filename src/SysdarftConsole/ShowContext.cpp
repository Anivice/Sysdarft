/* ShowContext.cpp
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

#include <iomanip>
#include <sstream>
#include <algorithm> // for std::min
#include <string>
#include <vector>
#include <cstdint>
#include <SysdarftCPU.h>
#include <InstructionSet.h>
#include <SysdarftMain.h>


std::string print_bin_line(const std::vector < uint8_t > & data)
{
    std::stringstream ss;
    for (auto it = data.begin(); it != data.end();)
    {
        ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(*it);
        ++it;
        if (it != data.end())
        {
            ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(*it) << " ";
            ++it;
        }
    }

    return ss.str();
}

std::string print_ascii(const std::vector < uint8_t > & data)
{
    std::stringstream ss;
    for (const auto & c : data)
    {
        if (c >= 0x20 && c <= 0x7E) {
            ss << static_cast<char>(c);
        } else {
            ss << '.';
        }
    }

    return ss.str();
}

std::string xxd_like_dump(const uint64_t offset, const std::vector<uint8_t>& data)
{
    constexpr std::size_t BYTES_PER_LINE = 16;
    std::vector < std::vector < uint8_t > > split_data;
    for (uint64_t i = 0; i < data.size() / BYTES_PER_LINE; ++i)
    {
        std::vector < uint8_t > line;
        line.resize(BYTES_PER_LINE);
        std::memcpy(line.data(), data.data() + (i * BYTES_PER_LINE), BYTES_PER_LINE);
        split_data.push_back(line);
    }

    if (data.size() % BYTES_PER_LINE != 0)
    {
        std::vector < uint8_t > line;
        line.resize(data.size() % BYTES_PER_LINE);
        std::memcpy(line.data(),
            data.data() + (data.size() / BYTES_PER_LINE) * BYTES_PER_LINE,
            data.size() % BYTES_PER_LINE);
        split_data.push_back(line);
    }

    uint64_t origin = offset;
    std::stringstream ss;
    for (const auto& line : split_data)
    {
        ss << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << origin << ": ";
        origin += line.size();
        const auto hex = print_bin_line(line);
        const auto ascii = print_ascii(line);
        const auto characters_per_line = BYTES_PER_LINE / 2 + BYTES_PER_LINE * 2;
        const auto characters_printed = line.size() / 2 + line.size() * 2;
        const std::string padding_space(characters_per_line - characters_printed, ' ');
        ss << hex << padding_space << "   " << ascii << std::endl;
    }

#ifdef __DEBUG__
    auto str = ss.str();
    return str;
#else
    return ss.str();
#endif
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

std::string current_instruction(const uint8_t opcode, const SysdarftCPU::WidthAndOperandsType & Arg)
{
    std::stringstream ss;

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW CURRENT INSTRUCTION PENDING TO BE EXECUTED
    ////////////////////////////////////////////////////////////////////////////////

    for (auto [fst, snd] : instruction_map) {
        if (snd.at(ENTRY_OPCODE) == opcode) {
            ss << fst << " ";
        }
    }

    switch (Arg.first) {
    case _8bit_prefix:  ss << ".8bit "; break;
    case _16bit_prefix: ss << ".16bit"; break;
    case _32bit_prefix: ss << ".32bit"; break;
    case _64bit_prefix: ss << ".64bit"; break;
    default: ; // fall through
    }

    int first = 0;
    for (const auto& op : Arg.second) {
        ss << " " << op.get_literal() << (first++ == 0 && Arg.second.size() > 1 ? "," : "");
    }

    return ss.str();
}

std::string show_context(SysdarftCPU &CPUInstance,
    uint64_t actual_ip,
    const uint8_t,
    const SysdarftCPU::WidthAndOperandsType &)
{
    std::stringstream ss;

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW ALL REGISTERS
    ////////////////////////////////////////////////////////////////////////////////

    ss << "==============================================================================\n";
    ss << "Registers:\n";
    // --- 8x 8-bit "RegisterType" registers (R0..R7) ---
    {
        auto R0 = CPUInstance.load<RegisterType, 0>();
        auto R1 = CPUInstance.load<RegisterType, 1>();
        auto R2 = CPUInstance.load<RegisterType, 2>();
        auto R3 = CPUInstance.load<RegisterType, 3>();
        auto R4 = CPUInstance.load<RegisterType, 4>();
        auto R5 = CPUInstance.load<RegisterType, 5>();
        auto R6 = CPUInstance.load<RegisterType, 6>();
        auto R7 = CPUInstance.load<RegisterType, 7>();

        ss << "R0 = 0x" + to_hex_string(R0) + " ";
        ss << "R1 = 0x" + to_hex_string(R1) + " ";
        ss << "R2 = 0x" + to_hex_string(R2) + " ";
        ss << "R3 = 0x" + to_hex_string(R3) + " ";
        ss << "R4 = 0x" + to_hex_string(R4) + " ";
        ss << "R5 = 0x" + to_hex_string(R5) + " ";
        ss << "R6 = 0x" + to_hex_string(R6) + " ";
        ss << "R7 = 0x" + to_hex_string(R7) + "\n";
    }

    // --- 8x 16-bit "ExtendedRegisterType" registers (EXR0..EXR7) ---
    {
        auto EXR0 = CPUInstance.load<ExtendedRegisterType, 0>();
        auto EXR1 = CPUInstance.load<ExtendedRegisterType, 1>();
        auto EXR2 = CPUInstance.load<ExtendedRegisterType, 2>();
        auto EXR3 = CPUInstance.load<ExtendedRegisterType, 3>();
        auto EXR4 = CPUInstance.load<ExtendedRegisterType, 4>();
        auto EXR5 = CPUInstance.load<ExtendedRegisterType, 5>();
        auto EXR6 = CPUInstance.load<ExtendedRegisterType, 6>();
        auto EXR7 = CPUInstance.load<ExtendedRegisterType, 7>();

        ss << "EXR0 = 0x" + to_hex_string(EXR0) + " ";
        ss << "EXR1 = 0x" + to_hex_string(EXR1) + " ";
        ss << "EXR2 = 0x" + to_hex_string(EXR2) + " ";
        ss << "EXR3 = 0x" + to_hex_string(EXR3) + "\n";
        ss << "EXR4 = 0x" + to_hex_string(EXR4) + " ";
        ss << "EXR5 = 0x" + to_hex_string(EXR5) + " ";
        ss << "EXR6 = 0x" + to_hex_string(EXR6) + " ";
        ss << "EXR7 = 0x" + to_hex_string(EXR7) + "\n";
    }

    // --- 8x 32-bit "HalfExtendedRegisterType" registers (HER0..HER7) ---
    {
        auto HER0 = CPUInstance.load<HalfExtendedRegisterType, 0>();
        auto HER1 = CPUInstance.load<HalfExtendedRegisterType, 1>();
        auto HER2 = CPUInstance.load<HalfExtendedRegisterType, 2>();
        auto HER3 = CPUInstance.load<HalfExtendedRegisterType, 3>();
        auto HER4 = CPUInstance.load<HalfExtendedRegisterType, 4>();
        auto HER5 = CPUInstance.load<HalfExtendedRegisterType, 5>();
        auto HER6 = CPUInstance.load<HalfExtendedRegisterType, 6>();
        auto HER7 = CPUInstance.load<HalfExtendedRegisterType, 7>();

        ss << "HER0 = 0x" + to_hex_string(HER0) + " ";
        ss << "HER1 = 0x" + to_hex_string(HER1) + " ";
        ss << "HER2 = 0x" + to_hex_string(HER2) + " ";
        ss << "HER3 = 0x" + to_hex_string(HER3) + "\n";
        ss << "HER4 = 0x" + to_hex_string(HER4) + " ";
        ss << "HER5 = 0x" + to_hex_string(HER5) + " ";
        ss << "HER6 = 0x" + to_hex_string(HER6) + " ";
        ss << "HER7 = 0x" + to_hex_string(HER7) + "\n";
    }

    // --- 16x 64-bit "FullyExtendedRegisterType" registers (FER0..FER15) ---
    {
        auto FER0  = CPUInstance.load<FullyExtendedRegisterType, 0>();
        auto FER1  = CPUInstance.load<FullyExtendedRegisterType, 1>();
        auto FER2  = CPUInstance.load<FullyExtendedRegisterType, 2>();
        auto FER3  = CPUInstance.load<FullyExtendedRegisterType, 3>();
        auto FER4  = CPUInstance.load<FullyExtendedRegisterType, 4>();
        auto FER5  = CPUInstance.load<FullyExtendedRegisterType, 5>();
        auto FER6  = CPUInstance.load<FullyExtendedRegisterType, 6>();
        auto FER7  = CPUInstance.load<FullyExtendedRegisterType, 7>();
        auto FER8  = CPUInstance.load<FullyExtendedRegisterType, 8>();
        auto FER9  = CPUInstance.load<FullyExtendedRegisterType, 9>();
        auto FER10 = CPUInstance.load<FullyExtendedRegisterType, 10>();
        auto FER11 = CPUInstance.load<FullyExtendedRegisterType, 11>();
        auto FER12 = CPUInstance.load<FullyExtendedRegisterType, 12>();
        auto FER13 = CPUInstance.load<FullyExtendedRegisterType, 13>();
        auto FER14 = CPUInstance.load<FullyExtendedRegisterType, 14>();
        auto FER15 = CPUInstance.load<FullyExtendedRegisterType, 15>();

        ss << "FER0  = 0x" + to_hex_string(FER0)  + "\n";
        ss << "FER1  = 0x" + to_hex_string(FER1)  + "\n";
        ss << "FER2  = 0x" + to_hex_string(FER2)  + "\n";
        ss << "FER3  = 0x" + to_hex_string(FER3)  + "\n";
        ss << "FER4  = 0x" + to_hex_string(FER4)  + "\n";
        ss << "FER5  = 0x" + to_hex_string(FER5)  + "\n";
        ss << "FER6  = 0x" + to_hex_string(FER6)  + "\n";
        ss << "FER7  = 0x" + to_hex_string(FER7)  + "\n";
        ss << "FER8  = 0x" + to_hex_string(FER8)  + "\n";
        ss << "FER9  = 0x" + to_hex_string(FER9)  + "\n";
        ss << "FER10 = 0x" + to_hex_string(FER10) + "\n";
        ss << "FER11 = 0x" + to_hex_string(FER11) + "\n";
        ss << "FER12 = 0x" + to_hex_string(FER12) + "\n";
        ss << "FER13 = 0x" + to_hex_string(FER13) + "\n";
        ss << "FER14 = 0x" + to_hex_string(FER14) + "\n";
        ss << "FER15 = 0x" + to_hex_string(FER15) + "\n";
    }

    // --- Flag Register (bitfield) ---
    {
        auto flags = CPUInstance.load<FlagRegisterType>();
        ss << "Flags:\n";
        ss << "  Carry            = " + std::to_string((int)flags.Carry) + "\n";
        ss << "  Overflow         = " + std::to_string((int)flags.Overflow) + "\n";
        ss << "  LargerThan       = " + std::to_string((int)flags.LargerThan) + "\n";
        ss << "  LessThan         = " + std::to_string((int)flags.LessThan) + "\n";
        ss << "  Equal            = " + std::to_string((int)flags.Equal) + "\n";
        ss << "  InterruptionMask = " + std::to_string((int)flags.InterruptionMask) + "\n";
    }

    // --- 64-bit registers: StackBase, StackPointer, CodeBase, InstructionPointer,
    //     DataBase, DataPointer, ExtendedBase, ExtendedPointer,
    //     CurrentProcedureStackPreservationSpace ---
    {
        auto sb  = CPUInstance.load<StackBaseType>();
        auto sp  = CPUInstance.load<StackPointerType>();
        auto cb  = CPUInstance.load<CodeBaseType>();
        auto rip = actual_ip;
        auto db  = CPUInstance.load<DataBaseType>();
        auto dp  = CPUInstance.load<DataPointerType>();
        auto eb  = CPUInstance.load<ExtendedBaseType>();
        auto ep  = CPUInstance.load<ExtendedPointerType>();
        auto cps = CPUInstance.load<CurrentProcedureStackPreservationSpaceType>();

        ss << "SB  = 0x" + to_hex_string(sb)  + "\n";
        ss << "SP  = 0x" + to_hex_string(sp)  + "\n";
        ss << "CB  = 0x" + to_hex_string(cb)  + "\n";
        ss << "IP  = 0x" + to_hex_string(rip) + "\n";
        ss << "DB  = 0x" + to_hex_string(db)  + "\n";
        ss << "DP  = 0x" + to_hex_string(dp)  + "\n";
        ss << "EB  = 0x" + to_hex_string(eb)  + "\n";
        ss << "EP  = 0x" + to_hex_string(ep)  + "\n";
        ss << "CPS = 0x" + to_hex_string(cps) + "\n";
    }

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW DB:DP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    try
    {
        ss << "==============================================================================\n";
        ss << "[DB:DP]:\n";
        auto data_off = CPUInstance.load<DataBaseType>() + CPUInstance.load<DataPointerType>();
        auto data_len = std::min(CPUInstance.SystemTotalMemory() - data_off, 128ul);
        std::vector<uint8_t> data_buffer;
        for (uint64_t i = 0; i < data_len; i++) {
            char c;
            CPUInstance.read_memory(data_off + i, &c, 1);
            data_buffer.push_back(c);
        }
        ss << xxd_like_dump(data_off, data_buffer) << "\n";
    } catch (const std::exception&) {
        ss << "[DB:DP]: [INVALID]\n";
    }

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW EB:EP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    try
    {
        ss << "==============================================================================\n";
        ss << "[EB:EP]:\n";
        auto ext_off = CPUInstance.load<ExtendedBaseType>() + CPUInstance.load<ExtendedPointerType>();
        auto ext_len = std::min(CPUInstance.SystemTotalMemory() - ext_off, 128ul);
        std::vector<uint8_t> ext_buffer;
        for (uint64_t i = 0; i < ext_len; i++) {
            char c;
            CPUInstance.read_memory(ext_off + i, &c, 1);
            ext_buffer.push_back(c);
        }
        ss << xxd_like_dump(ext_off, ext_buffer) << "\n";
    } catch (const std::exception&) {
        ss << "[EB:EP]: [INVALID]\n";
    }

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW SB:SP (128 bytes)
    ////////////////////////////////////////////////////////////////////////////////
    try
    {
        ss << "==============================================================================\n";
        ss << "[SB:SP]:\n";
        auto stack_off = CPUInstance.load<StackBaseType>() + CPUInstance.load<StackPointerType>();
        auto stack_len = std::min(CPUInstance.SystemTotalMemory() - stack_off, 128ul);
        std::vector<uint8_t> stack_buffer;
        for (uint64_t i = 0; i < stack_len; i++) {
            char c;
            CPUInstance.read_memory(stack_off + i, &c, 1);
            stack_buffer.push_back(c);
        }
        ss << xxd_like_dump(stack_off, stack_buffer) << "\n";
    } catch (const std::exception&) {
        ss << "[EB:SP]: [INVALID]\n";
    }

    ss << "==============================================================================\n";

    ////////////////////////////////////////////////////////////////////////////////
    // SHOW CURRENT AND NEXT 8 INSTRUCTIONS
    ////////////////////////////////////////////////////////////////////////////////
    auto show_next_8_instructions = [&]()
    {
        std::vector<std::string> next_8_instructions;
        const uint64_t offset = CPUInstance.load<CodeBaseType>() + actual_ip;
        const uint64_t length = std::min<uint64_t>(256, CPUInstance.SystemTotalMemory() - offset);
        std::vector<uint8_t> buffer_max256;
        buffer_max256.reserve(length);

        for (uint64_t i = 0; i < length; i++)
        {
            char c;
            CPUInstance.read_memory(offset + i, &c, 1);
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
            ss << instruction << "\n";
        }
    };

    show_next_8_instructions();

    // return result
    return ss.str();
}
