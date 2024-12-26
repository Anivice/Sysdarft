#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>

// User-defined literal for Hz (integer)
constexpr unsigned long long operator"" _Hz(unsigned long long freq) {
    return freq; // Return as unsigned long long
}

// Global frequency variable with Hz suffix
constexpr unsigned long long frequencyHz = 500_Hz; // Set to 1 kHz

// Calculate the period based on frequency (in nanoseconds)
constexpr std::chrono::nanoseconds period_ns(1'000'000'000 / frequencyHz);

// Define the maximum allowed duration for an operation based on frequency
constexpr std::chrono::nanoseconds MAX_DURATION_NS = period_ns;

// Sample operation function
void operation(int id)
{
    // Simulate variable operation duration
    // For demonstration, we'll randomly sleep between 500,000 ns to 1,500,000 ns
    // This range is fixed and does not depend on frequencyHz
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<long long> distribution(500'000, 1'000'000); // nanoseconds
    long long sleep_time_ns = distribution(generator);
    std::chrono::nanoseconds sleep_duration(sleep_time_ns);
    std::this_thread::sleep_for(sleep_duration);

    // Uncomment below to see operation details (might slow down execution)
    // std::cout << "Operation " << id << " completed in " << sleep_duration.count() << " ns.\n";
}

int main()
{
    // Calculate the number of operations per second based on frequency
    // Since frequencyHz is in Hz, operations per second = frequencyHz
    constexpr unsigned long long N = frequencyHz; // 1000 for 1 kHz

    std::cout << "Starting execution of " << N << " operations per second.\n";
    std::cout << "Frequency set to: " << frequencyHz << " Hz.\n";
    std::cout << "Period per operation: " << period_ns.count() << " nanoseconds.\n";
    std::cout << "Maximum allowed duration per operation: " << MAX_DURATION_NS.count() << " nanoseconds.\n\n";

    // Start time
    const auto start_time = std::chrono::steady_clock::now();

    // Counters for reporting
    long long overrun_count = 0;
    constexpr long long behind_schedule_count = 0;

    for (unsigned long long i = 0; i < N; ++i)
    {
        // Current time before operation
        auto op_start = std::chrono::steady_clock::now();

        // Execute the operation synchronously
        operation(static_cast<int>(i));

        // Current time after operation
        auto op_end = std::chrono::steady_clock::now();

        // Calculate the duration of the operation

        // Check if the operation exceeded the maximum allowed duration
        if (auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(op_end - op_start);
            duration > MAX_DURATION_NS)
        {
            std::cerr << "Warning: Operation " << i
                      << " exceeded the time limit of " << MAX_DURATION_NS.count()
                      << " ns. Actual duration: " << duration.count()
                      << " ns.\n";
            overrun_count++;
        }

        // Calculate the next scheduled time
        auto next_scheduled_time = start_time + ((i + 1) * period_ns);

        // Current time after operation

        if (auto now = std::chrono::steady_clock::now();
            next_scheduled_time > now)
        {
            // If we're ahead of schedule, wait until the next scheduled time
            std::this_thread::sleep_until(next_scheduled_time);
        }
    }

    // End time
    const auto end_time = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = end_time - start_time;

    // Reporting
    std::cout << "\nExecution Summary:\n";
    std::cout << "-------------------\n";
    std::cout << "Total operations executed: " << N << "\n";
    std::cout << "Total time elapsed: " << std::fixed << std::setprecision(6)
              << elapsed.count() << " seconds.\n";
    std::cout << "Operations that exceeded the time limit: " << overrun_count << "\n";
    std::cout << "Operations that were behind schedule: " << behind_schedule_count << "\n";

    return 0;
}
