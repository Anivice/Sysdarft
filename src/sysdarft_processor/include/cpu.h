#ifndef CPU_H
#define CPU_H

#include <worker.h>
#include <instance.h>

inline unsigned long long operator"" _Hz(const unsigned long long freq) {
    return freq;
}

class CPUConfigurationError final : public SysdarftBaseError {
public:
    explicit CPUConfigurationError(const std::string & msg) : SysdarftBaseError("CPU configuration error: " + msg) { }
};

class MultipleCPUInstanceCreation final : public SysdarftBaseError {
public:
    explicit MultipleCPUInstanceCreation() : SysdarftBaseError("Trying to create multiple CPU instances!") { }
};

class processor
{
private:
    void operation(__uint128_t timestamp, uint8_t current_core);

    void triggerer_thread(std::atomic<bool> & running,
        std::atomic<bool> & stopped);
    worker_thread triggerer;

    std::atomic<unsigned long long> frequencyHz = 1000_Hz;
    std::atomic<uint8_t> core_count = 2;
    std::atomic<double> wait_scale = 1.0;
    std::atomic<bool> has_instance = false;

public:
    void collaboration();
    void start_triggering();
    void stop_triggering();
    void push_code(const code_block_t & code_block);

    processor() : triggerer(this, &processor::triggerer_thread)
    {
        if (core_count == 0) {
            throw CPUConfigurationError("Core count can't be 0!");
        }
    }
};

#endif //CPU_H
