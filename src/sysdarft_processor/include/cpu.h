#ifndef CPU_H
#define CPU_H

#include <WorkerThread.h>
#include <instance.h>
#include <cstdint>
#include <memory>
#include <EncodingDecoding.h>
#include <register_def.h>

#define PAGE_SIZE 4096

inline unsigned long long operator"" _Hz(const unsigned long long freq) {
    return freq;
}

class CPUConfigurationError final : public SysdarftBaseError
{
public:
    explicit CPUConfigurationError(const std::string& msg) :
        SysdarftBaseError("CPU configuration error: " + msg) { }
};

class MultipleCPUInstanceCreation final : public SysdarftBaseError
{
public:
    explicit MultipleCPUInstanceCreation() :
        SysdarftBaseError("Trying to create multiple CPU instances!") { }
};

class IllegalMemoryAccessException final : public SysdarftBaseError {
public:
    explicit IllegalMemoryAccessException(const std::string & msg) :
        SysdarftBaseError("Illegal memory access: " + msg) { }
};

class IllegalCoreRegisterException final : public SysdarftBaseError {
public:
    explicit IllegalCoreRegisterException(const std::string & msg) :
        SysdarftBaseError("Illegal core register access: " + msg) { }
};

class IllegalInstruction final : public SysdarftBaseError {
public:
    explicit IllegalInstruction(const std::string & msg) :
        SysdarftBaseError("Illegal instruction: " + msg) { }
};

// Type trait to map SIZE to corresponding unsigned integer type
template <size_t SIZE>
struct size_to_uint;

// Specializations for supported sizes
template <>
struct size_to_uint<8> {
    using type = uint8_t;
};

template <>
struct size_to_uint<16> {
    using type = uint16_t;
};

template <>
struct size_to_uint<32> {
    using type = uint32_t;
};

template <>
struct size_to_uint<64> {
    using type = uint64_t;
};

// Type trait to map SIZE to corresponding signed integer type
template <size_t SIZE>
struct size_to_int;

// Specializations for supported sizes
template <>
struct size_to_int<8> {
    using type = int8_t;
};

template <>
struct size_to_int<16> {
    using type = int16_t;
};

template <>
struct size_to_int<32> {
    using type = int32_t;
};

template <>
struct size_to_int<64> {
    using type = int64_t;
};

// Define the NumType concept.
template<typename T>
concept NumType =
      std::same_as<T, int8_t>
   || std::same_as<T, uint8_t>
   || std::same_as<T, int16_t>
   || std::same_as<T, uint16_t>
   || std::same_as<T, int32_t>
   || std::same_as<T, uint32_t>
   || std::same_as<T, int64_t>
   || std::same_as<T, uint64_t>;

#define BIOS_START          0xC1800
#define BIOS_END            0xFFFFF
#define BIOS_SIZE           (BIOS_END - BIOS_START + 1)
#define BOOT_LOADER_START   0x00000
#define BOOT_LOADER_END     0x9FFFF
#define BOOT_LOADER_SIZE    (BOOT_LOADER_END - BOOT_LOADER_START + 1)
#define INTERRUPTION_VECTOR 0xA0000
#define INTERRUPTION_VEC_L  (INTERRUPTION_VECTOR + 512 * 8)

class SYSDARFT_EXPORT_SYMBOL processor
{
public:
    class Target;

private:
    // OK. A lot of definitions here and I simply can't do anything about it
    class __InstructionExecutorType__{
    private:
        processor & CPU;
        Target pop_target();

        void check_overflow(uint8_t bcd_width, __uint128_t val);
        static std::string bcd_width_str(uint8_t width);

        template < unsigned SIZE, typename ValueType = const char [SIZE] >
        void rlmode_push_stack(ValueType val)
        {
            uint64_t lowerbond = CPU.Registers.StackPointer;
            lowerbond -= SIZE;
            CPU.write_memory(lowerbond, val, SIZE);
            CPU.Registers.StackPointer = lowerbond;
        }

        template < unsigned SIZE, typename ValueType = const char [SIZE] >
        void rlmode_pop_stack(ValueType val)
        {
            uint64_t lowerbond = CPU.Registers.StackPointer;
            CPU.read_memory(lowerbond, val, SIZE);
            CPU.Registers.StackPointer = lowerbond + SIZE;
        }

    public:
        void nop(__uint128_t timestamp);
        void add(__uint128_t timestamp);
        void adc(__uint128_t timestamp);
        void sub(__uint128_t timestamp);
        void sbb(__uint128_t timestamp);
        void imul(__uint128_t timestamp);
        void mul(__uint128_t timestamp);
        void idiv(__uint128_t timestamp);
        void div(__uint128_t timestamp);
        void neg(__uint128_t timestamp);
        void cmp(__uint128_t timestamp);

        void and_(__uint128_t timestamp);
        void or_(__uint128_t timestamp);
        void xor_(__uint128_t timestamp);
        void not_(__uint128_t timestamp);
        void shl(__uint128_t timestamp);
        void shr(__uint128_t timestamp);
        void sal(__uint128_t timestamp);
        void sar(__uint128_t timestamp);
        void ror(__uint128_t timestamp);
        void rol(__uint128_t timestamp);
        void rcl(__uint128_t timestamp);
        void rcr(__uint128_t timestamp);

        void mov(__uint128_t timestamp);
        void xchg(__uint128_t timestamp);
        void push(__uint128_t timestamp);
        void pop(__uint128_t timestamp);
        void pushall(__uint128_t timestamp);
        void popall(__uint128_t timestamp);
        void enter(__uint128_t timestamp);
        void leave(__uint128_t timestamp);
        void movs(__uint128_t timestamp);

        explicit __InstructionExecutorType__(processor & _CPU) : CPU(_CPU) { }
    } InstructionExecutor;

    void output_debug_info();
    void operation(__uint128_t timestamp);
    void soft_interruption_ready(const uint64_t int_code);

    void triggerer_thread(std::atomic<bool>& running);
    WorkerThread triggerer;

    std::atomic<unsigned long long> frequencyHz = 1000_Hz;
    std::atomic<double> wait_scale = 1.0;
    std::atomic<bool> has_instance = false;
    std::atomic<bool> is_at_breakpoint = false;

    std::mutex RegisterAccessMutex;
    SysdarftRegister Registers;

    std::mutex MemoryAccessMutex;
    /*
     * Memory Layout:
     * 0x00000 - 0x9FFFF [BOOT CODE]     - 640KB
     * 0xA0000 - 0xC17FF [CONFIGURATION] - 134KB
     *                   [4KB Interruption Table: 512 Interrupts]
     * 0xC1800 - 0xFFFFF [FIRMWARE]      - 250KB
     */
    std::vector < std::array < unsigned char, PAGE_SIZE > > Memory;
    std::atomic<uint64_t> TotalMemory = 32 * 1024 * 1024; // 32MB Memory

    void initialize_registers();
    void initialize_memory();
    void read_memory(uint64_t address, char * _dest, uint64_t size);
    void write_memory(uint64_t address, const char* _source, uint64_t size);
    template <size_t SIZE> typename size_to_uint<SIZE>::type pop();

    template < unsigned int LENGTH >
    std::any rlmode_pop(const uint64_t begin, uint64_t & offset)
    {
        static_assert(LENGTH % 8 == 0);
        __uint128_t result = 0;
        auto buffer = (char*)(&result);
        read_memory(begin + offset, buffer, LENGTH / 8);
        offset += LENGTH / 8;
        switch (LENGTH / 8)
        {
        case 1: /* 8bit */  return static_cast<uint8_t> (result & 0xFF);
        case 2: /* 16bit */ return static_cast<uint16_t>(result & 0xFFFF);
        case 4: /* 32bit */ return static_cast<uint32_t>(result & 0xFFFFFFFF);
        case 8: /* 64bit */ return static_cast<uint64_t>(result & 0xFFFFFFFFFFFFFFFF);
        default: throw SysdarftCodeExpressionError("Unrecognized length");
        }
    }

    uint8_t rlmode_pop8(const uint64_t begin, uint64_t & offset) {
        return std::any_cast<uint8_t>(rlmode_pop<8>(begin, offset));
    }

    uint16_t rlmode_pop16(const uint64_t begin, uint64_t & offset) {
        return std::any_cast<uint16_t>(rlmode_pop<16>(begin, offset));
    }

    uint32_t rlmode_pop32(const uint64_t begin, uint64_t & offset) {
        return std::any_cast<uint32_t>(rlmode_pop<32>(begin, offset));
    }

    uint64_t rlmode_pop64(const uint64_t begin, uint64_t & offset) {
        return std::any_cast<uint64_t>(rlmode_pop<64>(begin, offset));
    }

    void rlmode_decode_constant(std::vector<std::string> & output, uint64_t begin, uint64_t & offset);
    void rlmode_decode_register(std::vector<std::string> & output, uint64_t begin, uint64_t & offset);
    void rlmode_decode_memory(std::vector<std::string> & output, uint64_t begin, uint64_t & offset);
    void rlmode_decode_target(std::vector<std::string> & output, uint64_t begin, uint64_t & offset);
    std::string rlmode_decode_instruction_within_range(uint64_t start, uint64_t length);
public:
    class Target {
    private:
        processor & CPU;

        enum { NaT /* Not a Type */,
            TypeRegister = REGISTER_PREFIX,
            TypeConstant = CONSTANT_PREFIX,
            TypeMemory = MEMORY_PREFIX,
        } TargetType { };

        uint8_t TargetWidth { };
        union {
            uint8_t RegisterIndex;
            uint64_t ConstantValue;
            uint64_t MemoryAddress; // Calculated Memory Address
        } TargetInformation { };

        std::string literal;

        void do_setup_register_info();
        void do_setup_constant_info();
        void do_setup_memory_info();
        void do_decode_via_prefix(uint8_t prefix);
        [[nodiscard]] uint64_t do_get_register();
        void do_set_register(uint64_t);
        [[nodiscard]] uint64_t get_target_content_in_u64bit_t();
        void set_target_content_in_u64bit_t(uint64_t);
        explicit Target(processor &);

    public:
        template < NumType ValueType > Target & operator=(const ValueType & val) {
            set_target_content_in_u64bit_t(val);
            return *this;
        }

        template < NumType ValueType > bool operator==(const ValueType & val) {
            return get_target_content_in_u64bit_t() == val;
        }

        template < NumType ValueType > ValueType get() {
            return get_target_content_in_u64bit_t();
        }

        Target & operator=(const Target &) = delete;
        ~Target() = default;

        friend class __InstructionExecutorType__;
    };

    void collaborate();
    void start_triggering();
    void stop_triggering();
    void load_bios(std::array<uint8_t, BIOS_SIZE> const &bios);

    void break_here() { is_at_breakpoint = true; }
    typedef void (*breakpoint_handler_t)(__uint128_t timestamp,
        SysdarftRegister & register_,
        decltype(Memory) & Memory_);
    std::atomic<breakpoint_handler_t> breakpoint_handler;

    processor() :
        InstructionExecutor(*this),
        triggerer(this, &processor::triggerer_thread)
    {
        initialize_registers();
        initialize_memory();
        breakpoint_handler = [](__uint128_t, decltype(Registers)&, decltype(Memory)& ) { };
    }

    friend class Target;
    friend class __InstructionExecutorType__;
};

#include "cpu.inl"

#define INT_FATAL_ERROR             0x000
#define INT_ILLEGAL_INSTRUCTION     0x001

#endif //CPU_H
