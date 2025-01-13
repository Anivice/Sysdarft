#ifndef SYSDARFTCPUINSTRUCTIONDECODER_H
#define SYSDARFTCPUINSTRUCTIONDECODER_H

#include <EncodingDecoding.h>
#include <SysdarftCursesUI.h>
#include <SysdarftDebug.h>
#include <SysdarftMemory.h>
#include <SysdarftRegister.h>

class IllegalInstruction final : public SysdarftBaseError {
public:
    explicit IllegalInstruction(const std::string & msg) :
        SysdarftBaseError("Illegal instruction: " + msg) { }
};

class SysdarftBadInterruption final : public SysdarftBaseError {
public:
    explicit SysdarftBadInterruption(const std::string & msg) : SysdarftBaseError("Bad interruption: " + msg) { }
};

class OperandType;

class DecoderDataAccess
    : public SysdarftRegister,
      public SysdarftCPUMemoryAccess,
      public SysdarftCursesUI
{
protected:
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
            struct {
                uint8_t RegisterWidthBCD;
                uint8_t RegisterIndex;
            } RegisterValue;

            struct {
                uint64_t MemoryAddress;
                uint8_t RegisterWidthBCD;
            } CalculatedMemoryAddress;
        } OperandInfo { };

        std::string literal; // literal will be wrong for all FPU and signed instructions
                             // since it decodes to unsigned only.
                             // These instructions have to output correct literals manually
    } OperandReferenceTable { };

    uint64_t do_access_register_based_on_table();

    template < typename DataType >
    DataType do_width_ambiguous_access_memory_based_on_table()
    {
        const auto DB = Access.load<DataBaseType>();
        auto DP = OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress;
        return Access.pop_memory_from<DataType>(DB, DP);
    }

    uint64_t do_access_width_specified_access_memory_based_on_table()
    {
        switch (OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.RegisterWidthBCD) {
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

    uint64_t do_access_operand_based_on_table();
    void store_value_to_operand_based_on_table(uint64_t value);

public:
    [[nodiscard]] uint64_t get_val() { return do_access_operand_based_on_table(); }
    [[nodiscard]] uint64_t get_effective_addr() const { return OperandReferenceTable.OperandInfo.CalculatedMemoryAddress.MemoryAddress; }
    void set_val(const uint64_t val) { store_value_to_operand_based_on_table(val); }
    [[nodiscard]] std::string get_literal() const { return OperandReferenceTable.literal; }
    explicit OperandType(DecoderDataAccess & Access_) : Access(Access_) { do_decode_operand(); }
};

class SysdarftInterruptionOutOfRange final : public SysdarftBaseError {
public:
    explicit SysdarftInterruptionOutOfRange(const std::string & msg) : SysdarftBaseError("Interruption out of range: " + msg) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPUInterruption : public DecoderDataAccess
{
private:
    struct InterruptionPointer {
        uint64_t InterruptionTargetCodeBase;
        uint64_t InterruptionTargetInstructionPointer;
    };

    /*
     * Interruption table:
     * Hardware Reserved:
     *  [0x00] FATAL ERROR (ErrorCode == %EXR0)
     *  [0x01] DIV/0
     *  [0x02] IO ERROR
     *  [0x03] DEBUG, BREAKPOINT RIGHT NEXT
     *  [0x04] BAD INTERRUPTION CALL
     *  [0x05]
     *  [0x06]
     *  [0x07]
     *  [0x08]
     *  [0x09]
     *  [0x0A]
     *  [0x0B]
     *  [0x0C]
     *  [0x0D]
     *  [0x0E]
     *  [0x0F]
     *  [0x10] TELETYPE (EXR0 == Character ASCII Code)
     *  [0x11] SET CURSOR POSITION (EXR0 == LinearOffset)
     *  [0x12] SET CURSOR VISIBILITY (EXR0 == Visibility)
     *  [0x13] NEW LINE
     *  [0x14] GET INPUT, INPUT == EXR0
     *  [0x15] GET CURSOR POSITION POSITION == EXR0
     *  [0x16]
     *  [0x17]
     *  [0x18]
     *  [0x19]
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
    void do_interruption_tty_0x10();
    void do_interruption_set_cur_pos_0x11();
    void do_interruption_set_cur_visib_0x12();
    void do_interruption_newline_0x13();
    void do_interruption_getinput_0x14();
    void do_interruption_cur_pos_0x15();

protected:
    std::atomic<bool> hd_int_flag = false;

    void do_interruption(uint64_t code);
    void do_iret();
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
};

#endif //SYSDARFTCPUINSTRUCTIONDECODER_H