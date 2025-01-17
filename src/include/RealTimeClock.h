#ifndef REALTIMECLOCK_H
#define REALTIMECLOCK_H

#define RTC_CURRENT_TIME  (0x70UL)
#define RTC_SET_INTERRUPT (0x71UL) /* [63-39] Reserved */
                                 /* [38-9] * 5000ns (0.005 ms) */
                                 /* [8-0] interruption number, <= 0x1F means disable interruption */

#include <SysdarftIOHub.h>
#include <WorkerThread.h>
#include <SysdarftCPU.h>

class SysdarftRealTimeClock final : public SysdarftExternalDeviceBaseClass
{
private:
    decltype(std::chrono::system_clock::now()) m_startTime;
    decltype(std::chrono::system_clock::now()) m_machineTime;
    SysdarftCPU & m_cpu;
    std::mutex m_mutex;
    WorkerThread update_time_worker;
    std::atomic < uint64_t > interruption_number = 0;
    std::atomic < uint64_t > interruption_scale = 0;

    void update_time(std::atomic < bool > & running);

public:
    explicit SysdarftRealTimeClock(SysdarftCPU & _instance);
    ~SysdarftRealTimeClock() override { update_time_worker.stop(); }
    bool request_read(uint64_t port) override;
    bool request_write(uint64_t port) override;
};

#endif //REALTIMECLOCK_H
