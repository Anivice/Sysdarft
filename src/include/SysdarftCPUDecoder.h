/* SysdarftCPUDecoder.h
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

#ifndef SYSDARFTCPUINSTRUCTIONDECODER_H
#define SYSDARFTCPUINSTRUCTIONDECODER_H

#include <EncodingDecoding.h>
#include <SysdarftCursesUI.h>
#include <SysdarftDebug.h>
#include <SysdarftMemory.h>
#include <SysdarftRegister.h>

#define INT_FATAL                   (0x00)
#define INT_DIV_0                   (0x01)
#define INT_IO_ERROR                (0x02)
#define INT_DEBUG                   (0x03)
#define INT_BAD_INTR                (0x04)
#define INT_ABORT                   (0x05)
#define INT_ILLEGAL_INSTRUCTION     (0x06)
#define INT_STACKOVERFLOW           (0x07)
#define INT_ILLEGAL_MEMORY_ACCESS   (0x08)
#define INT_SYSTEM_SHUTDOWN         (0x09)

#define INT_TELETYPE    (0x10)
#define INT_SET_CUR_POS (0x11)
#define INT_SET_CUR_VSB (0x12)
#define INT_NEWLINE     (0x13)
#define INT_GET_INPUT   (0x14)
#define INT_GET_CUR_POS (0x15)
#define INT_GET_HD_INFO (0x16)

class IllegalInstruction final : public SysdarftBaseError {
public:
    explicit IllegalInstruction(const std::string & msg) :
        SysdarftBaseError("Illegal instruction: " + msg) { }
};

class SysdarftBadInterruption final : public SysdarftBaseError {
public:
    explicit SysdarftBadInterruption(const std::string & msg) : SysdarftBaseError("Bad interruption: " + msg) { }
};

class SysdarftCPUFatal final : public SysdarftBaseError {
public:
    SysdarftCPUFatal() : SysdarftBaseError("CPU is met with a unrecoverable fatal error") { }
};

class SysdarftCPUInitializeFailed final : public SysdarftBaseError {
public:
    SysdarftCPUInitializeFailed() : SysdarftBaseError("Setup is met with a unrecoverable fatal error") { }
};

class SysdarftInterruptionOutOfRange final : public SysdarftBaseError {
public:
    explicit SysdarftInterruptionOutOfRange(const std::string & msg) : SysdarftBaseError("Interruption out of range: " + msg) { }
};

class OperandType;

class DecoderDataAccess
    : public SysdarftRegister,
      public SysdarftCursesUI
{
protected:
    explicit DecoderDataAccess(const uint64_t memory, const std::string & font_name) : SysdarftCursesUI(memory, font_name) { }

    template < typename DataType >
    DataType pop_code_and_inc_ip()
    {
        DataType result { };
        const auto CB = SysdarftRegister::load<CodeBaseType>();
        auto IP = SysdarftRegister::load<InstructionPointerType>();
        result = pop_memory_from<DataType>(CB, IP);
        SysdarftRegister::store<InstructionPointerType>(IP);
        return result;
    }

    uint8_t pop_code8() {
        return pop_code_and_inc_ip<uint8_t>();
    }

    uint16_t pop_code16() {
        return pop_code_and_inc_ip<uint16_t>();
    }

    uint32_t pop_code32() {
        return pop_code_and_inc_ip<uint32_t>();
    }

    uint64_t pop_code64() {
        return pop_code_and_inc_ip<uint64_t>();
    }

public:
    friend class OperandType;
};

// short-lived type, valid only for current timestamp
class SYSDARFT_EXPORT_SYMBOL OperandType
{
protected:
    DecoderDataAccess & Access;

    enum OperandType_t { NaO, RegisterOperand, ConstantOperand, MemoryOperand };

    struct
    {
        OperandType_t OperandType;

        struct {
            uint64_t ConstantValue;
            uint8_t ConstantWidth;
            struct {
                uint8_t RegisterWidthBCD;
                uint8_t RegisterIndex;
            } RegisterValue;

            struct {
                uint64_t MemoryAddress;
                uint8_t MemoryWidthBCD;
            } CalculatedMemoryAddress;
        } OperandInfo { };

        std::string literal; // literal will be wrong for all FPU and signed instructions
                             // since it decodes to unsigned only.
                             // These instructions have to output correct literals manually
    } OperandReferenceTable { };

    [[nodiscard]] uint64_t do_access_register_based_on_table() const;

    template < typename DataType >
    [[nodiscard]] DataType do_width_ambiguous_access_memory_based_on_table() const
    {
        auto DP = OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress;
        return Access.pop_memory_from<DataType>(0, DP);
    }

    [[nodiscard]] uint64_t do_access_width_specified_access_memory_based_on_table() const
    {
        switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryWidthBCD) {
        case _8bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint8_t>();
        case _16bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint16_t>();
        case _32bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint32_t>();
        case _64bit_prefix: return do_width_ambiguous_access_memory_based_on_table<uint64_t>();
        default: throw IllegalInstruction("Unknown memory access width");
        }
    }

    void store_value_to_register_based_on_table(uint64_t value);
    void store_value_to_memory_based_on_table(uint64_t value);

    void do_decode_register_without_prefix();
    void do_decode_constant_without_prefix();
    void do_decode_memory_without_prefix();
    void do_decode_operand();

    [[nodiscard]] uint64_t do_access_operand_based_on_table() const;
    void store_value_to_operand_based_on_table(uint64_t value);

public:
    [[nodiscard]] uint64_t get_val() const { return do_access_operand_based_on_table(); }
    [[nodiscard]] uint64_t get_effective_addr() const { return OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress; }
    void set_val(const uint64_t val) { store_value_to_operand_based_on_table(val); }
    [[nodiscard]] std::string get_literal() const { return OperandReferenceTable.literal; }
    explicit OperandType(DecoderDataAccess & Access_) : Access(Access_) { do_decode_operand(); }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInterruption : public DecoderDataAccess
{
protected:
    std::atomic < bool > debugger_pause_blocked_int_0x14 = false;
    std::mutex External_Int_Req_Vec_Protector;
    std::vector < uint64_t > interruption_requests;
    std::atomic < bool > external_device_requested = false;
    std::atomic < uint64_t > current_routine_pop_len = 0;
    std::atomic < uint64_t > ip_before_pop = 0;

    struct InterruptionPointer {
        uint64_t InterruptionTargetCodeBase;
        uint64_t InterruptionTargetInstructionPointer;
    };

    /*
     * Interruption table:
     * Hardware Reserved:
     *      [1-F], INVOKED BY HARDWARE TO INDICATE AN ERROR, NOT MASKABLE
     *  [0x00] FATAL ERROR (ErrorCode == %EXR0)
     *  [0x01] DIV/0
     *  [0x02] IO ERROR, EXR0 == 0xF0, IO ERROR, EXR0 == 0xF1, NO SUCH DEVICE
     *  [0x03] DEBUG, BREAKPOINT RIGHT NEXT
     *  [0x04] BAD INTERRUPTION CALL
     *  [0x05] ABORT
     *  [0x06] ILLEGAL INSTRUCTION
     *  [0x07] STACK OVERFLOW
     *  [0x08] MEMORY ACCESS OUT OF BOUNDARY
     *  [0x09] SYSTEM SHUTDOWN (Can only trigger once until IM is 0)
     *  [0x0A]
     *  [0x0B]
     *  [0x0C]
     *  [0x0D]
     *  [0x0E]
     *  [0x0F]
     *      [10-1F], INVOKED BY USER FOR SPECIFIC HARDWARE FUNCTIONS, NOT MASKABLE
     *  [0x10] TELETYPE (EXR0 == Character ASCII Code)
     *  [0x11] SET CURSOR POSITION (EXR0 == LinearOffset)
     *  [0x12] SET CURSOR VISIBILITY (EXR0 == Visibility)
     *  [0x13] NEW LINE
     *  [0x14] GET INPUT, INPUT == EXR0
     *  [0x15] GET CURSOR POSITION == EXR0
     *  [0x16] GET SYSTEM HARDWARE INFO [TOTAL MEMORY == FER0]
     *  [0x17] RING BELL
     *  [0x18] REFRESH
     *  [0x19] CLEAR USER KEYBOARD INPUT STREAM
     *  [0x1A]
     *  [0x1B]
     *  [0x1C]
     *  [0x1D]
     *  [0x1E]
     *  [0x1F]
     */

    InterruptionPointer do_interruption_lookup(uint64_t code);
    void do_preserve_cpu_state();
    void do_jump_table(const InterruptionPointer & location);

    // Hardware Interruptions
    void do_interruption_fatal_0x00();
    void do_interruption_debug_0x03();
    void do_stackoverflow_0x07();
    void do_abort_0x05();
    void do_interruption_tty_0x10();
    void do_interruption_set_cur_pos_0x11();
    void do_interruption_set_cur_visib_0x12();
    void do_interruption_newline_0x13();
    void do_interruption_getInput_0x14();
    void do_interruption_cur_pos_0x15();
    void do_get_system_hardware_info_0x16();
    void do_ring_bell_0x17();
    void do_refresh_screen_0x18();
    void do_clear_user_input_stream_0x19();

    std::atomic<bool> Int3DebugInterrupt = false;

    void do_interruption(uint64_t code);
    void do_iret();

    explicit SysdarftCPUInterruption(uint64_t memory, const std::string & font_name);
private:

    void set_mask()
    {
        auto fg = SysdarftRegister::load<FlagRegisterType>();
        fg.InterruptionMask = 1;
        SysdarftRegister::store<FlagRegisterType>(fg);
    }

public:
    void do_ext_dev_interruption(uint64_t code);
    void debugger_pause_0x14() { debugger_pause_blocked_int_0x14 = true;}
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInstructionDecoder : public SysdarftCPUInterruption
{
protected:
    struct ActiveInstructionType {
        uint8_t opcode;
        uint8_t width;
        std::vector< OperandType > operands;
        std::string literal; // again, literals for FPU and signed operations are all wrong
                             // it will remain this way to reduce the complicity
                             // manually output these instruction literals
                             // instead of relying on automatic literalization
    };

    ActiveInstructionType pop_instruction_from_ip_and_increase_ip();

    explicit SysdarftCPUInstructionDecoder(const uint64_t total_memory, const std::string & font_name)
        : SysdarftCPUInterruption(total_memory, font_name) { }
};

#endif //SYSDARFTCPUINSTRUCTIONDECODER_H
