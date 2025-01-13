#include <chrono>
#include <thread>
#include <WorkerThread.h>
#include <cpu.h>

void processor::triggerer_thread(std::atomic<bool> & running)
{
    const std::chrono::nanoseconds period_ns(1'000'000'000 / frequencyHz);
    const std::chrono::nanoseconds MAX_DURATION_NS = period_ns + std::chrono::nanoseconds();
    const unsigned long long ops_num = frequencyHz;

    log("[CPU] Starting execution of ", ops_num, " operations per second.\n");
    log("[CPU] Frequency set to: ", frequencyHz, " Hz.\n");
    log("[CPU] Period per operation: ", period_ns.count(), " nanoseconds.\n");
    log("[CPU] Maximum allowed duration per operation: ", MAX_DURATION_NS.count(), " nanoseconds.\n");

    __uint128_t timestamp = 0;

    while (running)
    {
        auto op_start = std::chrono::steady_clock::now();

        operation(timestamp);

        auto op_end = std::chrono::steady_clock::now();
        if (const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(op_end - op_start);
            duration > MAX_DURATION_NS) {
            log("[CPU] Delayed ", duration - MAX_DURATION_NS, " for current cycle!\n");
        } else {
            std::this_thread::sleep_for(MAX_DURATION_NS * wait_scale.load() - duration);
        }

        timestamp++;
    }
}

void processor::collaborate()
{
    log("[CPU] CPU time frame precision collaboration started.\n");
    log("[CPU] Please wait for 10 seconds for the collaboration procedure to finish!\n");

    std::atomic<unsigned long long int> ins_count = 0;
    std::atomic<bool> running = true;
    std::atomic<bool> stopped = false;

    auto minimum_operation = [&ins_count]() {
        ++ins_count;
    };

    auto triggerer = [&]()->void
    {
        const std::chrono::nanoseconds period_ns(1'000'000'000 / frequencyHz);
        const std::chrono::nanoseconds MAX_DURATION_NS = period_ns + std::chrono::nanoseconds();

        __uint128_t timestamp = 0;

        while (running)
        {
            auto op_start = std::chrono::steady_clock::now();

            minimum_operation();

            auto op_end = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(op_end - op_start);

            if (duration > MAX_DURATION_NS) {
                log("[CPU] Delayed ", duration - MAX_DURATION_NS, " ns for current cycle!\n");
            } else {
                std::this_thread::sleep_for(MAX_DURATION_NS - duration);
            }

            timestamp++;
        }

        stopped = true;
    };

    std::thread T1(triggerer);
    T1.detach();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    running = false;
    while (!stopped) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    const auto expected = frequencyHz * 10;
    const uint64_t actual = ins_count;

    wait_scale = static_cast<double>(actual) / static_cast<double>(expected);

    log("[CPU] CPU time frame precision collaboration done.\n");
}

void processor::start_triggering()
{
    if (has_instance) {
        throw MultipleCPUInstanceCreation();
    }
    has_instance = true;
    triggerer.start();
}

void processor::stop_triggering()
{
    if (has_instance)
    {
        triggerer.stop();
        has_instance = false;
    }
}
