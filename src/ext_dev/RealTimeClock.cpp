#include <RealTimeClock.h>

SysdarftRealTimeClock::SysdarftRealTimeClock(SysdarftCPU & _instance)
: m_cpu(_instance), update_time_worker(this, &SysdarftRealTimeClock::update_time)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    device_buffer.emplace(RTC_CURRENT_TIME,  std::make_unique<ControllerDataStream>());
    device_buffer.emplace(RTC_SET_INTERRUPT, std::make_unique<ControllerDataStream>());
    m_startTime = std::chrono::system_clock::now();
    m_machineTime = std::chrono::system_clock::now();
    update_time_worker.start();
}

bool SysdarftRealTimeClock::request_read(const uint64_t port)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto timeElapsedSinceStart = std::chrono::system_clock::now() - m_machineTime;
    const uint64_t RTCTime = std::chrono::duration_cast<std::chrono::seconds>
        ((m_startTime + timeElapsedSinceStart).time_since_epoch()).count();

    if (port == RTC_CURRENT_TIME) {
        device_buffer.at(port)->push(RTCTime);
        return true;
    }

    return false;
}

bool SysdarftRealTimeClock::request_write(const uint64_t port)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (port == RTC_CURRENT_TIME)
    {
        const auto epoch_seconds = device_buffer.at(port)->pop<uint64_t>();
        m_startTime = std::chrono::system_clock::time_point{std::chrono::seconds(epoch_seconds)};
        return true;
    }

    if (port == RTC_SET_INTERRUPT)
    {
        const auto data = device_buffer.at(port)->pop<uint64_t>();
        const uint64_t int_num = data & 0x1FF;
        const uint64_t int_scale = (data >> 9) & 0x3FFFFFFF; /* approximately 1 hour, 29 minutes, and 28.7 seconds max */

        if (int_num <= 0x1F || int_num >= 512) {
            return false;
        }

        if (int_scale == 0) {
            return false;
        }

        interruption_scale = int_scale;
        interruption_number = int_num;
        return true;
    }

    return false;
}

void SysdarftRealTimeClock::update_time(std::atomic < bool > & running)
{
    debug::set_thread_name("RTC");
    __uint128_t scale_count = interruption_scale;

    while (running)
    {
        const auto before = std::chrono::system_clock::now();
        {
            if (scale_count != 0) {
                scale_count--;
            } else {
                scale_count = interruption_scale;
                try {
                    if (interruption_number > 0x1F && interruption_number < 512) {
                        m_cpu.do_ext_dev_interruption(interruption_number);
                    }
                } catch (...) {
                    // TL;DR: error occurred, aborting further interruption notice.
                    // Long comment:
                    // note that this has nothing to do with internal software error.
                    // this should only capture system instruction procedure abortion
                    // and any unexpected errors.
                    // either way, it will either damage stack frame and render the entire interruption useless.
                    // or cause sysdarft to quit entirely
                    interruption_number = 0;
                }
            }
        }

        const auto after = std::chrono::system_clock::now();

        if (const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before);
            duration.count() < 5000)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(5000 - duration.count()));
        }
    }
}
