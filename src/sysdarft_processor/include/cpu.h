#ifndef CPU_H
#define CPU_H

#include <worker.h>
#include <instance.h>
#include <cstdint>
#include <memory>

// Width
#define _8bit_prefix  0x08
#define _16bit_prefix 0x16
#define _32bit_prefix 0x32
#define _64bit_prefix 0x64
#define FLOATING_POINT_PREFIX 0xFC

// Prefix
#define REGISTER_PREFIX 0x01
#define CONSTANT_PREFIX 0x02
#define MEMORY_PREFIX   0x03

// Specific Registers
#define R_StackPointer                     0xA0
#define R_StackConfiguration               0xA1
#define R_CodeConfiguration                0xA2
#define R_DataPointer                      0xA3
#define R_DataConfiguration                0xA4
#define R_ExtendedSegmentPointer           0xA5
#define R_ExtendedSegmentConfiguration     0xA6
#define R_SegmentationAccessTable          0xA7
#define R_ControlRegister0                 0xA8

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

struct alignas(8) sysdarft_register_t
{
    struct SegmentationConfigurationRegister
    {
        uint64_t BaseAddress;
        uint64_t AddressLimit;

        struct
        {
            struct
            {
                uint32_t __reserved : 32;
            } ControlFlags;

            struct
            {
                uint32_t __reserved : 32;
            } AccessFlags;
        } ConfigurationFlags;

        uint64_t __reserved;
    };

    struct
    {
        struct
        {
            struct
            {
                uint8_t R0;
                uint8_t R1;
            } ExtendedRegister0;

            struct
            {
                uint8_t R2;
                uint8_t R3;
            } ExtendedRegister1;
        } HalfExtendedRegister0;

        struct
        {
            struct
            {
                uint8_t R4;
                uint8_t R5;
            } ExtendedRegister2;

            struct
            {
                uint8_t R6;
                uint8_t R7;
            } ExtendedRegister3;
        } HalfExtendedRegister1;
    } FullyExtendedRegister0;

    struct
    {
        struct
        {
            uint16_t ExtendedRegister4;
            uint16_t ExtendedRegister5;
        } HalfExtendedRegister2;

        struct
        {
            uint16_t ExtendedRegister6;
            uint16_t ExtendedRegister7;
        } HalfExtendedRegister3;
    } FullyExtendedRegister1;

    struct
    {
        uint32_t HalfExtendedRegister4;
        uint32_t HalfExtendedRegister5;
    } FullyExtendedRegister2;

    struct
    {
        uint32_t HalfExtendedRegister6;
        uint32_t HalfExtendedRegister7;
    } FullyExtendedRegister3;

    uint64_t FullyExtendedRegister4;
    uint64_t FullyExtendedRegister5;
    uint64_t FullyExtendedRegister6;
    uint64_t FullyExtendedRegister7;
    uint64_t FullyExtendedRegister8;
    uint64_t FullyExtendedRegister9;
    uint64_t FullyExtendedRegister10;
    uint64_t FullyExtendedRegister11;
    uint64_t FullyExtendedRegister12;
    uint64_t FullyExtendedRegister13;
    uint64_t FullyExtendedRegister14;
    uint64_t FullyExtendedRegister15;

    struct
    {
        uint64_t Carry : 1;
        uint64_t Overflow : 1;
        uint64_t InterruptionMask : 1;
        uint64_t CurrentPrivilegeLevel : 8;
        uint64_t __reserved : 53;
    } FlagRegister;

    struct {
        uint64_t ProtectedMode : 1;
        uint64_t Paging : 1;
        uint64_t __reserved:62;
    } ControlRegister0;

    uint64_t StackPointer;
    SegmentationConfigurationRegister StackConfiguration;
    uint64_t InstructionPointer;
    SegmentationConfigurationRegister CodeConfiguration;
    uint64_t DataPointer;
    SegmentationConfigurationRegister DataConfiguration;
    uint64_t ExtendedSegmentPointer;
    SegmentationConfigurationRegister ExtendedSegmentConfiguration;

    struct
    {
        uint64_t PointerBaseAddress;
        uint64_t PointerLimit;
    } SegmentationAccessTablePointer;

    struct
    {
        long double XMM0;
        long double XMM1;
        long double XMM2;
        long double XMM3;
        long double XMM4;
        long double XMM5;
        long double XMM6;
        long double XMM7;
    } FPURegister;
};

class SysdarftRegister
{
protected:
    sysdarft_register_t Registers{};

public:
    uint8_t& R0 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R0;
    uint8_t& R1 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R1;
    uint8_t& R2 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R2;
    uint8_t& R3 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R3;
    uint8_t& R4 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R4;
    uint8_t& R5 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R5;
    uint8_t& R6 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R6;
    uint8_t& R7 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R7;

    uint16_t& ExtendedRegister0 = *(uint16_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister0.
                                                          ExtendedRegister0);
    uint16_t& ExtendedRegister1 = *(uint16_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister0.
                                                          ExtendedRegister1);
    uint16_t& ExtendedRegister2 = *(uint16_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister1.
                                                          ExtendedRegister2);
    uint16_t& ExtendedRegister3 = *(uint16_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister1.
                                                          ExtendedRegister3);
    uint16_t& ExtendedRegister4 = Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister4;
    uint16_t& ExtendedRegister5 = Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister5;
    uint16_t& ExtendedRegister6 = Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister6;
    uint16_t& ExtendedRegister7 = Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister7;

    uint32_t& HalfExtendedRegister0 = *(uint32_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister0);
    uint32_t& HalfExtendedRegister1 = *(uint32_t*)(&Registers.FullyExtendedRegister0.HalfExtendedRegister1);
    uint32_t& HalfExtendedRegister2 = *(uint32_t*)(&Registers.FullyExtendedRegister1.HalfExtendedRegister2);
    uint32_t& HalfExtendedRegister3 = *(uint32_t*)(&Registers.FullyExtendedRegister1.HalfExtendedRegister3);
    uint32_t& HalfExtendedRegister4 = Registers.FullyExtendedRegister2.HalfExtendedRegister4;
    uint32_t& HalfExtendedRegister5 = Registers.FullyExtendedRegister2.HalfExtendedRegister5;
    uint32_t& HalfExtendedRegister6 = Registers.FullyExtendedRegister3.HalfExtendedRegister6;
    uint32_t& HalfExtendedRegister7 = Registers.FullyExtendedRegister3.HalfExtendedRegister7;

    uint64_t& FullyExtendedRegister0 = *(uint64_t*)(&Registers.FullyExtendedRegister0);
    uint64_t& FullyExtendedRegister1 = *(uint64_t*)(&Registers.FullyExtendedRegister1);
    uint64_t& FullyExtendedRegister2 = *(uint64_t*)(&Registers.FullyExtendedRegister2);
    uint64_t& FullyExtendedRegister3 = *(uint64_t*)(&Registers.FullyExtendedRegister3);
    uint64_t& FullyExtendedRegister4 = Registers.FullyExtendedRegister4;
    uint64_t& FullyExtendedRegister5 = Registers.FullyExtendedRegister5;
    uint64_t& FullyExtendedRegister6 = Registers.FullyExtendedRegister6;
    uint64_t& FullyExtendedRegister7 = Registers.FullyExtendedRegister7;
    uint64_t& FullyExtendedRegister8 = Registers.FullyExtendedRegister8;
    uint64_t& FullyExtendedRegister9 = Registers.FullyExtendedRegister9;
    uint64_t& FullyExtendedRegister10 = Registers.FullyExtendedRegister10;
    uint64_t& FullyExtendedRegister11 = Registers.FullyExtendedRegister11;
    uint64_t& FullyExtendedRegister12 = Registers.FullyExtendedRegister12;
    uint64_t& FullyExtendedRegister13 = Registers.FullyExtendedRegister13;
    uint64_t& FullyExtendedRegister14 = Registers.FullyExtendedRegister14;
    uint64_t& FullyExtendedRegister15 = Registers.FullyExtendedRegister15;

    decltype(Registers.FlagRegister)& FlagRegister = Registers.FlagRegister;

    uint64_t& StackPointer = Registers.StackPointer;
    sysdarft_register_t::SegmentationConfigurationRegister& StackConfiguration = Registers.StackConfiguration;
    uint64_t& InstructionPointer = Registers.InstructionPointer;
    sysdarft_register_t::SegmentationConfigurationRegister& CodeConfiguration = Registers.CodeConfiguration;
    uint64_t& DataPointer = Registers.DataPointer;
    sysdarft_register_t::SegmentationConfigurationRegister& DataConfiguration = Registers.DataConfiguration;
    uint64_t& ExtendedSegmentPointer = Registers.ExtendedSegmentPointer;
    sysdarft_register_t::SegmentationConfigurationRegister& ExtendedSegmentConfiguration
        = Registers.ExtendedSegmentConfiguration;
    decltype(Registers.SegmentationAccessTablePointer)& SegmentationAccessTable
        = Registers.SegmentationAccessTablePointer;

    decltype(Registers.ControlRegister0) & ControlRegister0 = Registers.ControlRegister0;

    long double& XMM0 = Registers.FPURegister.XMM0;
    long double& XMM1 = Registers.FPURegister.XMM1;
    long double& XMM2 = Registers.FPURegister.XMM2;
    long double& XMM3 = Registers.FPURegister.XMM3;
    long double& XMM4 = Registers.FPURegister.XMM4;
    long double& XMM5 = Registers.FPURegister.XMM5;
    long double& XMM6 = Registers.FPURegister.XMM6;
    long double& XMM7 = Registers.FPURegister.XMM7;

    SysdarftRegister & operator=(const SysdarftRegister & other) = delete;
};

#define PAGE_SIZE 4096

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

// Define the NumType concept
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

class EXPORT processor
{
public:
    class Target;

private:
    // OK. A lot of definitions here and I simply can't do anything about it
    class __InstructionExecutorType__{
    private:
        processor & CPU;
        Target pop_target();

    public:
        void nop();
        void add();
        void pushall();

        explicit __InstructionExecutorType__(processor & _CPU) : CPU(_CPU) { }
    } InstructionExecutor;

    void operation(__uint128_t timestamp);
    void soft_interruption_ready(const uint64_t int_code);

    void triggerer_thread(std::atomic<bool>& running,
                          std::atomic<bool>& stopped);
    worker_thread triggerer;

    std::atomic<unsigned long long> frequencyHz = 1000_Hz;
    std::atomic<double> wait_scale = 1.0;
    std::atomic<bool> has_instance = false;

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
    void get_memory(uint64_t address, char * _dest, uint64_t size);
    void write_memory(uint64_t address, const char* _source, uint64_t size);
    template <size_t SIZE> typename size_to_uint<SIZE>::type pop();

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

    void collaboration();
    void start_triggering();
    void stop_triggering();
    void load_bios(std::array<uint8_t, BIOS_SIZE> const &bios);

    processor() :
        InstructionExecutor(*this),
        triggerer(this, &processor::triggerer_thread)
    {
        initialize_registers();
        initialize_memory();
    }

    friend class Target;
    friend class __InstructionExecutorType__;
};

#include "cpu.inl"

#define INT_FATAL_ERROR             0x000
#define INT_ILLEGAL_INSTRUCTION     0x001

#endif //CPU_H
