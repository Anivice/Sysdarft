#ifndef REGISTER_DEF_H
#define REGISTER_DEF_H

#include <cstdint>

struct alignas(16) sysdarft_register_t
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
    uint8_t& Register0 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R0;
    uint8_t& Register1 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R1;
    uint8_t& Register2 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R2;
    uint8_t& Register3 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R3;
    uint8_t& Register4 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R4;
    uint8_t& Register5 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R5;
    uint8_t& Register6 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R6;
    uint8_t& Register7 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R7;

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

#endif //REGISTER_DEF_H
