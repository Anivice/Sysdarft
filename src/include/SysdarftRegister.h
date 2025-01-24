/* SysdarftRegister.h
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

#ifndef REGISTER_DEF_H
#define REGISTER_DEF_H

#include <cstdint>
#include <SysdarftDebug.h>
#include <SysdarftMemory.h>

class UnknownRegisterIdentification final : public SysdarftBaseError {
public:
    explicit UnknownRegisterIdentification(const std::string & regName)
    : SysdarftBaseError("Unknown register " + regName) { }
};

// struct SelectorType {
//     uint64_t AddressStart; // not paged, linear, paged, page number
//     uint64_t AddressLimit; // not paged, linear, paged, page count
//     uint64_t IsAddressSpaceExpandUpwards:1;
//     uint64_t IsAddressPaged:1;
//     uint64_t
// };

struct alignas(16) sysdarft_register_t
{
    struct { struct { struct {
            uint8_t R0;
            uint8_t R1;
        } ExtendedRegister0;
        struct {
            uint8_t R2;
            uint8_t R3;
        } ExtendedRegister1;
        } HalfExtendedRegister0;

        struct { struct {
            uint8_t R4;
            uint8_t R5;
        } ExtendedRegister2;
        struct {
            uint8_t R6;
            uint8_t R7;
        } ExtendedRegister3;
        } HalfExtendedRegister1;
    } FullyExtendedRegister0;

    struct { struct {
        uint16_t ExtendedRegister4;
        uint16_t ExtendedRegister5;
        } HalfExtendedRegister2;
        struct {
            uint16_t ExtendedRegister6;
            uint16_t ExtendedRegister7;
        } HalfExtendedRegister3;
    } FullyExtendedRegister1;

    struct {
        uint32_t HalfExtendedRegister4;
        uint32_t HalfExtendedRegister5;
    } FullyExtendedRegister2;

    struct {
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
        uint64_t LargerThan: 1;
        uint64_t LessThan: 1;
        uint64_t Equal: 1;
        uint64_t InterruptionMask : 1;
        // unused:
        uint64_t CurrentPrivilegeLevel:2; // 0 == Real Mode Level, 1 == Protected Kernel Level, 2 == User Mode Level, 3 == Hypervisor Level
        uint64_t ProtectedModeEnabled:1;
        uint64_t PagingEnabled:1;
        uint64_t _reserved : 54;
    } FlagRegister;

    uint64_t StackBase;
    uint64_t StackPointer;
    uint64_t CodeBase;
    uint64_t InstructionPointer;
    uint64_t DataBase;
    uint64_t DataPointer;
    uint64_t ExtendedBase;
    uint64_t ExtendedPointer;
    uint64_t CurrentProcedureStackPreservationSpace;

    struct {
        uint64_t SelectorSpaceStart;
        uint64_t SelectorSPaceSize;
        uint64_t R_reserved1;
        uint64_t R_reserved2;
        uint64_t R_reserved3;
        uint64_t R_reserved4;
    } ProtectedModeRegisters;
};

class FullyExtendedRegisterType { };
class HalfExtendedRegisterType { };
class ExtendedRegisterType { };
class RegisterType { };
class FlagRegisterType { };
class StackBaseType { };
class StackPointerType { };
class CodeBaseType { };
class InstructionPointerType { };
class DataBaseType { };
class DataPointerType { };
class ExtendedBaseType { };
class ExtendedPointerType { };
class CurrentProcedureStackPreservationSpaceType { };
class BaseSelector { };
class BaseSelectorSize { };
class WholeRegisterType { };

template < typename AccessRegisterType >
struct RegisterTypeIdentifier;

template <>
struct RegisterTypeIdentifier<FullyExtendedRegisterType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<HalfExtendedRegisterType> {
    using type = uint32_t;
};

template <>
struct RegisterTypeIdentifier<ExtendedRegisterType> {
    using type = uint16_t;
};

template <>
struct RegisterTypeIdentifier<RegisterType> {
    using type = uint8_t;
};

template <>
struct RegisterTypeIdentifier<BaseSelector> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<BaseSelectorSize> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<FlagRegisterType> {
    using type = decltype(sysdarft_register_t::FlagRegister);
};

template <>
struct RegisterTypeIdentifier<StackBaseType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<StackPointerType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<CodeBaseType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<InstructionPointerType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<DataBaseType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<DataPointerType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<ExtendedBaseType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<ExtendedPointerType> {
    using type = uint64_t;
};

template <>
struct RegisterTypeIdentifier<CurrentProcedureStackPreservationSpaceType> {
    using type = uint64_t;
};


template <>
struct RegisterTypeIdentifier<WholeRegisterType> {
    using type = sysdarft_register_t;
};

class SysdarftRegister
{
protected:
    sysdarft_register_t Registers { };
    // in any case, Registers should only be exposed to **ONE AND ONLY ONE** thread.
    // that means no hyper-thread in this CPU (for now?)
    std::mutex RegisterModificationMutex;

public:
    template < typename AccessRegisterType, unsigned AccessRegisterIndex = 0 >
    requires std::is_same_v<AccessRegisterType, FullyExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, HalfExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, ExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, RegisterType>
    || std::is_same_v<AccessRegisterType, BaseSelector>
    || std::is_same_v<AccessRegisterType, BaseSelectorSize>
    || std::is_same_v<AccessRegisterType, FlagRegisterType>
    || std::is_same_v<AccessRegisterType, StackBaseType>
    || std::is_same_v<AccessRegisterType, StackPointerType>
    || std::is_same_v<AccessRegisterType, CodeBaseType>
    || std::is_same_v<AccessRegisterType, InstructionPointerType>
    || std::is_same_v<AccessRegisterType, DataBaseType>
    || std::is_same_v<AccessRegisterType, DataPointerType>
    || std::is_same_v<AccessRegisterType, ExtendedBaseType>
    || std::is_same_v<AccessRegisterType, ExtendedPointerType>
    || std::is_same_v<AccessRegisterType, WholeRegisterType>
    || std::is_same_v<AccessRegisterType, CurrentProcedureStackPreservationSpaceType>
    typename RegisterTypeIdentifier < AccessRegisterType >::type load()
    {
        std::lock_guard<std::mutex> lock(RegisterModificationMutex);
        if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 0) {
            auto FER0 = Registers.FullyExtendedRegister0;
            return *(uint64_t*)(&FER0);
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 1) {
            auto FER1 = Registers.FullyExtendedRegister1;
            return *(uint64_t*)(&FER1);
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 2) {
            auto FER2 = Registers.FullyExtendedRegister2;
            return *(uint64_t*)(&FER2);
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 3) {
            auto FER3 = Registers.FullyExtendedRegister3;
            return *(uint64_t*)(&FER3);
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 4) {
            return Registers.FullyExtendedRegister4;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 5) {
            return Registers.FullyExtendedRegister5;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 6) {
            return Registers.FullyExtendedRegister6;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 7) {
            return Registers.FullyExtendedRegister7;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 8) {
            return Registers.FullyExtendedRegister8;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 9) {
            return Registers.FullyExtendedRegister9;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 10) {
            return Registers.FullyExtendedRegister10;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 11) {
            return Registers.FullyExtendedRegister11;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 12) {
            return Registers.FullyExtendedRegister12;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 13) {
            return Registers.FullyExtendedRegister13;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 14) {
            return Registers.FullyExtendedRegister14;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 15) {
            return Registers.FullyExtendedRegister15;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 0) {
            auto HER0 = Registers.FullyExtendedRegister0.HalfExtendedRegister0;
            return *(uint32_t*)(&HER0);
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 1) {
            auto HER1 = Registers.FullyExtendedRegister0.HalfExtendedRegister1;
            return *(uint32_t*)(&HER1);
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 2) {
            auto HER2 = Registers.FullyExtendedRegister1.HalfExtendedRegister2;
            return *(uint32_t*)(&HER2);
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 3) {
            auto HER3 = Registers.FullyExtendedRegister1.HalfExtendedRegister3;
            return *(uint32_t*)(&HER3);
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 4) {
            return Registers.FullyExtendedRegister2.HalfExtendedRegister4;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 5) {
            return Registers.FullyExtendedRegister2.HalfExtendedRegister5;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 6) {
            return Registers.FullyExtendedRegister3.HalfExtendedRegister6;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 7) {
            return Registers.FullyExtendedRegister3.HalfExtendedRegister7;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 0) {
            auto EXR0 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0;
            return *(uint16_t*)(&EXR0);
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 1) {
            auto EXR1 = Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1;
            return *(uint16_t*)(&EXR1);
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 2) {
            auto EXR2 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2;
            return *(uint16_t*)(&EXR2);
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 3) {
            auto EXR3 = Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3;
            return *(uint16_t*)(&EXR3);
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 4) {
            return Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister4;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 5) {
            return Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister5;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 6) {
            return Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister6;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 7) {
            return Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister7;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 0) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R0;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 1) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R1;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 2) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R2;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 3) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R3;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 4) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R4;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 5) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R5;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 6) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R6;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 7) {
            return Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R7;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, BaseSelector> && AccessRegisterIndex == 0) {
            return Registers.ProtectedModeRegisters.SelectorSpaceStart;
        } else if constexpr (std::is_same_v<AccessRegisterType, BaseSelectorSize> && AccessRegisterIndex == 0) {
            return Registers.ProtectedModeRegisters.SelectorSPaceSize;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, FlagRegisterType>) {
            return Registers.FlagRegister;
        } else if constexpr (std::is_same_v<AccessRegisterType, StackBaseType>) {
            return Registers.StackBase;
        } else if constexpr (std::is_same_v<AccessRegisterType, StackPointerType>) {
            return Registers.StackPointer;
        } else if constexpr (std::is_same_v<AccessRegisterType, CodeBaseType>) {
            return Registers.CodeBase;
        } else if constexpr (std::is_same_v<AccessRegisterType, InstructionPointerType>) {
            return Registers.InstructionPointer;
        } else if constexpr (std::is_same_v<AccessRegisterType, DataBaseType>) {
            return Registers.DataBase;
        } else if constexpr (std::is_same_v<AccessRegisterType, DataPointerType>) {
            return Registers.DataPointer;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedBaseType>) {
            return Registers.ExtendedBase;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedPointerType>) {
            return Registers.ExtendedPointer;
        } else if constexpr (std::is_same_v<AccessRegisterType, WholeRegisterType>) {
            return Registers;
        } else if constexpr (std::is_same_v<AccessRegisterType, CurrentProcedureStackPreservationSpaceType>) {
            return Registers.CurrentProcedureStackPreservationSpace;
        } else {
#ifdef __DEBUG__
            throw UnknownRegisterIdentification("Invalid register type: "
                + std::string(typeid(AccessRegisterType).name()));
#else
            throw UnknownRegisterIdentification("Invalid register type");
#endif
        }
    }

    template < typename AccessRegisterType, unsigned AccessRegisterIndex = 0 >
    requires std::is_same_v<AccessRegisterType, FullyExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, HalfExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, ExtendedRegisterType>
    || std::is_same_v<AccessRegisterType, RegisterType>
    || std::is_same_v<AccessRegisterType, BaseSelector>
    || std::is_same_v<AccessRegisterType, BaseSelectorSize>
    || std::is_same_v<AccessRegisterType, FlagRegisterType>
    || std::is_same_v<AccessRegisterType, StackBaseType>
    || std::is_same_v<AccessRegisterType, StackPointerType>
    || std::is_same_v<AccessRegisterType, CodeBaseType>
    || std::is_same_v<AccessRegisterType, InstructionPointerType>
    || std::is_same_v<AccessRegisterType, DataBaseType>
    || std::is_same_v<AccessRegisterType, DataPointerType>
    || std::is_same_v<AccessRegisterType, ExtendedBaseType>
    || std::is_same_v<AccessRegisterType, ExtendedPointerType>
    || std::is_same_v<AccessRegisterType, WholeRegisterType>
    || std::is_same_v<AccessRegisterType, CurrentProcedureStackPreservationSpaceType>
    void store(const typename RegisterTypeIdentifier < AccessRegisterType >::type Reg)
    {
        std::lock_guard<std::mutex> lock(RegisterModificationMutex);
        if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 0) {
            *(uint64_t*)&Registers.FullyExtendedRegister0 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 1) {
            *(uint64_t*)&Registers.FullyExtendedRegister1 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 2) {
            *(uint64_t*)&Registers.FullyExtendedRegister2 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 3) {
            *(uint64_t*)&Registers.FullyExtendedRegister3 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 4) {
            Registers.FullyExtendedRegister4 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 5) {
            Registers.FullyExtendedRegister5 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 6) {
            Registers.FullyExtendedRegister6 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 7) {
            Registers.FullyExtendedRegister7 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 8) {
            Registers.FullyExtendedRegister8 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 9) {
            Registers.FullyExtendedRegister9 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 10) {
            Registers.FullyExtendedRegister10 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 11) {
            Registers.FullyExtendedRegister11 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 12) {
            Registers.FullyExtendedRegister12 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 13) {
            Registers.FullyExtendedRegister13 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 14) {
            Registers.FullyExtendedRegister14 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, FullyExtendedRegisterType> && AccessRegisterIndex == 15) {
            Registers.FullyExtendedRegister15 = Reg;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 0) {
            *(uint32_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister0 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 1) {
            *(uint32_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister1 = Reg;;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 2) {
            *(uint32_t*)&Registers.FullyExtendedRegister1.HalfExtendedRegister2 = Reg;;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 3) {
            *(uint32_t*)&Registers.FullyExtendedRegister1.HalfExtendedRegister3 = Reg;;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 4) {
            Registers.FullyExtendedRegister2.HalfExtendedRegister4 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 5) {
            Registers.FullyExtendedRegister2.HalfExtendedRegister5 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 6) {
            Registers.FullyExtendedRegister3.HalfExtendedRegister6 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, HalfExtendedRegisterType> && AccessRegisterIndex == 7) {
            Registers.FullyExtendedRegister3.HalfExtendedRegister7 = Reg;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 0) {
            *(uint16_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 1) {
            *(uint16_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 2) {
            *(uint16_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 3) {
            *(uint16_t*)&Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 4) {
            Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister4 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 5) {
            Registers.FullyExtendedRegister1.HalfExtendedRegister2.ExtendedRegister5 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 6) {
            Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister6 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedRegisterType> && AccessRegisterIndex == 7) {
            Registers.FullyExtendedRegister1.HalfExtendedRegister3.ExtendedRegister7 = Reg;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 0) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R0 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 1) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister0.R1 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 2) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R2 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 3) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister0.ExtendedRegister1.R3 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 4) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R4 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 5) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister2.R5 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 6) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R6 = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, RegisterType> && AccessRegisterIndex == 7) {
            Registers.FullyExtendedRegister0.HalfExtendedRegister1.ExtendedRegister3.R7 = Reg;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, BaseSelector> && AccessRegisterIndex == 0) {
            Registers.ProtectedModeRegisters.SelectorSpaceStart = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, BaseSelectorSize> && AccessRegisterIndex == 0) {
            Registers.ProtectedModeRegisters.SelectorSPaceSize = Reg;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if constexpr (std::is_same_v<AccessRegisterType, FlagRegisterType>) {
            Registers.FlagRegister = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, StackBaseType>) {
            Registers.StackBase = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, StackPointerType>) {
            Registers.StackPointer = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, CodeBaseType>) {
            Registers.CodeBase = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, InstructionPointerType>) {
            Registers.InstructionPointer = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, DataBaseType>) {
            Registers.DataBase = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, DataPointerType>) {
            Registers.DataPointer = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedBaseType>) {
            Registers.ExtendedBase = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, ExtendedPointerType>) {
            Registers.ExtendedPointer = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, WholeRegisterType>) {
            Registers = Reg;
        } else if constexpr (std::is_same_v<AccessRegisterType, CurrentProcedureStackPreservationSpaceType>) {
            Registers.CurrentProcedureStackPreservationSpace = Reg;
        } else {
#ifdef __DEBUG__
            throw UnknownRegisterIdentification("Invalid register type [AccessRegisterType = "
                + std::string(typeid(AccessRegisterType).name())
                + ", AccessRegisterIndex = " + std::to_string(AccessRegisterIndex));
#else
            throw UnknownRegisterIdentification("Invalid register type");
#endif
        }
    }

protected:
    SysdarftRegister() {
        store<InstructionPointerType>(BIOS_START);
    }

public:
    virtual ~SysdarftRegister() = default;
    SysdarftRegister & operator=(const SysdarftRegister & other) = delete;
};

#endif //REGISTER_DEF_H
