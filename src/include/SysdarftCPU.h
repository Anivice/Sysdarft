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
    std::atomic < __uint128_t > timestamp;

public:
    explicit SysdarftCPU(uint64_t memory,
        const std::vector < uint8_t > & bios,
        const std::string & hdd,
        const std::string & fda,
        const std::string & fdb);
    ~SysdarftCPU() override { SysdarftCursesUI::cleanup(); }

    void set_abort_next() { do_abort_int = true; }
    void system_hlt() { SystemHalted = true; }
    void Boot();

    SysdarftCPU & operator = (const SysdarftCPU &) = delete;
    [[nodiscard]] uint64_t SystemTotalMemory() const { return TotalMemory; }

    template < typename DeviceType, typename ... Args,
        typename = std::enable_if_t<std::is_base_of_v<SysdarftExternalDeviceBaseClass, DeviceType> > >
    void add_device(Args&... args)
    {
        device_list.emplace_back(std::make_unique<DeviceType>(args...));
    }
};

#endif //CPU_H
