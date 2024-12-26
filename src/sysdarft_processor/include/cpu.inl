#ifndef __CPU_INL__
#define __CPU_INL__

#include "cpu.h"

// Template pop function
template <size_t SIZE>
typename size_to_uint<SIZE>::type processor::pop(const uint8_t CoreID)
{
    static_assert(SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64,
          "Unsupported SIZE. Supported sizes are 8, 16, 32, 64.");

    if (CoreID >= core_count - 1) {
        throw IllegalCoreRegisterException("Core ID out of range");
    }

    using ReturnType = typename size_to_uint<SIZE>::type;

    ReturnType value = 0;
    const auto this_register = real_mode_register_access(CoreID);

    if (const auto control_register0 = real_mode_register_access(0).ControlRegister0;
        !control_register0.ProtectedMode)
    {
        // real mode
        get_memory(this_register.InstructionPointer, (char*)&value, SIZE / 8);
    }
    else
    {
        // protected mode
        if (this_register.InstructionPointer < this_register.CodeConfiguration.AddressLimit) {
            get_memory(this_register.CodeConfiguration.BaseAddress + this_register.InstructionPointer,
                (char*)&value, SIZE / 8);
        } else {
            throw IllegalMemoryAccessException("InstructionPointer out of range");
        }
    }

    this_register.InstructionPointer++;
    real_mode_register_store(this_register, CoreID);

    return value;
}

#endif // __CPU_INL__
