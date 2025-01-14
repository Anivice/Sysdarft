#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <memory>
#include <EncodingDecoding.h>
#include <SysdarftRegister.h>
#include <SysdarftInstructionExec.h>
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

#endif //CPU_H
