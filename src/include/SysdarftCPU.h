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
private:
    __uint128_t timestamp;

public:
    explicit SysdarftCPU(uint64_t memory,
        const std::vector < uint8_t > & bios,
        const std::string & hdd,
        const std::string & fda,
        const std::string & fdb);

    void Boot();
};

#endif //CPU_H
