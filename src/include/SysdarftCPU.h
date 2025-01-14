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

class SYSDARFT_EXPORT_SYMBOL SysdarftCPU final : public SysdarftCPUInstructionExecutor {
private:
    __uint128_t timestamp;
    std::atomic < bool > do_abort_int = false;

public:
    explicit SysdarftCPU(uint64_t memory,
        const std::vector < uint8_t > & bios,
        const std::string & hdd,
        const std::string & fda,
        const std::string & fdb);

    void set_abort_next() { do_abort_int = true; }
    void system_hlt() { SystemHalted = true; }
    void Boot();

    SysdarftCPU & operator = (const SysdarftCPU &) = delete;
};

#endif //CPU_H
