#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <memory>
#include <EncodingDecoding.h>
#include <SysdarftRegister.h>
#include <SysdarftInstructionExec.h>
#include <WorkerThread.h>

class MultipleCPUInstanceCreation final : public SysdarftBaseError
{
public:
    explicit MultipleCPUInstanceCreation() :
        SysdarftBaseError("Trying to create multiple CPU instances!") { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftCPU final : protected SysdarftCPUInstructionExecutor {
public:
    explicit SysdarftCPU(const uint64_t memory, std::vector < uint8_t > bios) :
        SysdarftCPUInstructionExecutor(memory) { }
};

#endif //CPU_H
