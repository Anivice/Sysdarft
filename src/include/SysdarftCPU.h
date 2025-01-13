#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <memory>
#include <EncodingDecoding.h>
#include <SysdarftRegister.h>
#include <WorkerThread.h>

inline unsigned long long operator"" _Hz(const unsigned long long freq) {
    return freq;
}

class MultipleCPUInstanceCreation final : public SysdarftBaseError
{
public:
    explicit MultipleCPUInstanceCreation() :
        SysdarftBaseError("Trying to create multiple CPU instances!") { }
};

#define INT_FATAL_ERROR             0x000
#define INT_ILLEGAL_INSTRUCTION     0x001

#endif //CPU_H
