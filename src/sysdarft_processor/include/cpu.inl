#ifndef __CPU_INL__
#define __CPU_INL__

#include "cpu.h"

// Template pop function
template <size_t SIZE>
typename size_to_uint<SIZE>::type processor::pop()
{
    std::lock_guard<std::mutex> lock(RegisterAccessMutex);
    static_assert(SIZE == 8 || SIZE == 16 || SIZE == 32 || SIZE == 64,
          "Unsupported SIZE. Supported sizes are 8, 16, 32, 64.");

    using ReturnType = typename size_to_uint<SIZE>::type;

    ReturnType value = 0;

    if (const auto control_register0 = Registers.ControlRegister0;
        !control_register0.ProtectedMode)
    {
        // real mode
        get_memory(Registers.InstructionPointer, (char*)&value, SIZE / 8);
    }
    else
    {
        // protected mode
        if (Registers.InstructionPointer < Registers.CodeConfiguration.AddressLimit) {
            get_memory(Registers.CodeConfiguration.BaseAddress + Registers.InstructionPointer,
                (char*)&value, SIZE / 8);
        } else {
            throw IllegalMemoryAccessException("Instruction Pointer out of range");
        }
    }

    Registers.InstructionPointer += SIZE / 8;

    return value;
}

template <size_t SIZE>
typename size_to_uint<SIZE>::type target_access()
{

}
template <size_t SIZE> void target_store(const typename size_to_uint<SIZE>::type &);

#endif // __CPU_INL__
