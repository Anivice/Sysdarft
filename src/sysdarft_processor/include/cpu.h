#ifndef CPU_H
#define CPU_H

#include <worker.h>
#include <instance.h>
#include <cstdint>

inline unsigned long long operator"" _Hz(const unsigned long long freq)
{
    return freq;
}

class CPUConfigurationError final : public SysdarftBaseError
{
public:
    explicit CPUConfigurationError(const std::string& msg) : SysdarftBaseError("CPU configuration error: " + msg)
    {
    }
};

class MultipleCPUInstanceCreation final : public SysdarftBaseError
{
public:
    explicit MultipleCPUInstanceCreation() : SysdarftBaseError("Trying to create multiple CPU instances!")
    {
    }
};

struct
    sysdarft_register_t
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

    // Only effect for CPU 0
    decltype(Registers.ControlRegister0) & ControlRegister0 = Registers.ControlRegister0;

    long double& XMM0 = Registers.FPURegister.XMM0;
    long double& XMM1 = Registers.FPURegister.XMM1;
    long double& XMM2 = Registers.FPURegister.XMM2;
    long double& XMM3 = Registers.FPURegister.XMM3;
    long double& XMM4 = Registers.FPURegister.XMM4;
    long double& XMM5 = Registers.FPURegister.XMM5;
    long double& XMM6 = Registers.FPURegister.XMM6;
    long double& XMM7 = Registers.FPURegister.XMM7;

    SysdarftRegister & operator=(const SysdarftRegister & other);
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

class processor
{
private:
    // OK. A lot of definitions here and I simply can't do anything about it
    class __InstructionExecutorType__{
    public:
        void nop();
        void pushall();
    } InstructionExecutor;

    void operation(__uint128_t timestamp, uint8_t current_core);
    void soft_interruption_ready(uint8_t current_core, const uint64_t int_code);

    void triggerer_thread(std::atomic<bool>& running,
                          std::atomic<bool>& stopped);
    worker_thread triggerer;

    std::atomic<unsigned long long> frequencyHz = 1000_Hz;
    std::atomic<uint8_t> core_count = 2;
    std::atomic<double> wait_scale = 1.0;
    std::atomic<bool> has_instance = false;

    std::mutex RegisterAccessMutex;
    std::vector < SysdarftRegister > Registers;

    std::mutex MemoryAccessMutex;
    std::vector < std::array < unsigned char, PAGE_SIZE > > Memory;
    std::atomic<uint64_t> TotalMemory = 32 * 1024 * 1024; // 32MB Memory

    void initialize_registers();
    void initialize_memory();
    void get_memory(uint64_t address, char * _dest, uint64_t size) const;
    void write_memory(uint64_t address, const char* _source, uint64_t size);

    SysdarftRegister real_mode_register_access(const uint8_t RegisterID);
    void real_mode_register_store(const SysdarftRegister&, uint8_t RegisterID);
    template <size_t SIZE> typename size_to_uint<SIZE>::type pop(const uint8_t CoreID);
    template <size_t SIZE> typename size_to_uint<SIZE>::type target_access(const uint8_t CoreID);

public:
    void collaboration();
    void start_triggering();
    void stop_triggering();

    processor() : triggerer(this, &processor::triggerer_thread)
    {
        if (core_count == 0)
        {
            throw CPUConfigurationError("Core count can't be 0!");
        }
    }
};

#include "cpu.inl"

#define INT_FATAL_ERROR             0x000
#define INT_ILLEGAL_INSTRUCTION     0x001

#endif //CPU_H
