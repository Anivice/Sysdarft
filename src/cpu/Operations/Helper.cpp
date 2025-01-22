#include <SysdarftInstructionExec.h>

uint64_t SysdarftCPUInstructionExecutor::check_overflow(const uint8_t BCDWidth, const __uint128_t Value)
{
    uint64_t compliment;

    switch (BCDWidth) {
    case _8bit_prefix: compliment = 0xFF; break;
    case _16bit_prefix: compliment = 0xFFFF; break;
    case _32bit_prefix: compliment = 0xFFFFFFFF; break;
    case _64bit_prefix: compliment = 0xFFFFFFFFFFFFFFFF; break;
    default: throw IllegalInstruction("Invalid operation width");
    }

    if ((Value & compliment) != Value) { // overflow
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 1; // unsigned overflow
        FG.Overflow = 0;
        FG.Equal = 0;
        FG.LargerThan = 0;
        FG.LessThan = 0;
        SysdarftRegister::store<FlagRegisterType>(FG);
    } else {
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 0; // unsigned overflow
        FG.Overflow = 0;
        FG.Equal = 0;
        FG.LargerThan = 0;
        FG.LessThan = 0;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }

    return Value & compliment;
}

// Primary template (left undefined)
template <unsigned SIZE>
struct IntType;

// Template specializations for supported sizes
template <>
struct IntType<0x08> {
    using type = int8_t;
    static constexpr uint64_t mask = 0xFF;
    static constexpr type minVal = -128;
    static constexpr type maxVal = 127;
};

template <>
struct IntType<0x16> {
    using type = int16_t;
    static constexpr uint64_t mask = 0xFFFF;
    static constexpr type minVal = -32768;
    static constexpr type maxVal = 32767;
};

template <>
struct IntType<0x32> {
    using type = int32_t;
    static constexpr uint64_t mask = 0xFFFFFFFF;
    static constexpr type minVal = -2147483648;
    static constexpr type maxVal = 2147483647;
};

template <>
struct IntType<0x64> {
    using type = int64_t;
    static constexpr uint64_t mask = 0xFFFFFFFFFFFFFFFF;
    static constexpr type minVal = (static_cast<__int128_t>(1ULL) << 63) * -1;
    static constexpr type maxVal = ((static_cast<__int128_t>(1ULL) << 63) - 1);
};

template <unsigned SIZE>
typename IntType<SIZE>::type check_overflow_signed(uint64_t val, bool & flag)
{
    const __int128_t ret = *(int64_t*)(&val);

    if (ret > IntType<SIZE>::maxVal || ret < IntType<SIZE>::minVal) {
        flag = true;
    } else {
        flag = false;
    }

    val &= IntType<SIZE>::mask;
    return *(typename IntType<SIZE>::type *)&val;
}

uint64_t SysdarftCPUInstructionExecutor::check_overflow_signed(
    const uint8_t BCDWidth,
    const __uint128_t Value)
{
    bool overflow;
    int64_t ret;
    int8_t r;

    // 1) figure out how many bits we’re dealing with and the mask
    switch (BCDWidth) {
    case _8bit_prefix:
        r = ::check_overflow_signed<0x08>(Value, overflow);
        ret = *(uint8_t*)&r;
        break;
    case _16bit_prefix:
        ret = ::check_overflow_signed<0x16>(Value, overflow);
        break;
    case _32bit_prefix:
        ret = ::check_overflow_signed<0x32>(Value, overflow);
        break;
    case _64bit_prefix:
        ret = ::check_overflow_signed<0x64>(Value, overflow);
        break;
    default:
        throw IllegalInstruction("Invalid operation width");
    }

    // 3) Check the range
    if (overflow)
    {
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 0;
        FG.Overflow = 1;
        FG.Equal = 0;
        FG.LargerThan = 0;
        FG.LessThan = 0;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }
    else
    {
        auto FG = SysdarftRegister::load<FlagRegisterType>();
        FG.Carry = 0;
        FG.Overflow = 0;
        FG.Equal = 0;
        FG.LargerThan = 0;
        FG.LessThan = 0;
        SysdarftRegister::store<FlagRegisterType>(FG);
    }

    return *(uint64_t*)&ret;
}
