#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>

// Your SysdarftCPUMemoryAccess header
#include <SysdarftMemory.h>

// A derived class that uses SysdarftCPUMemoryAccess
class Memory final : public SysdarftCPUMemoryAccess
{
public:
    Memory() : SysdarftCPUMemoryAccess(32 * 1024 * 1024) { }

    const uint64_t TotalMemory = SysdarftCPUMemoryAccess::TotalMemory;

    // Random write helper
    void random_write(std::mt19937 &gen,
                      std::uniform_int_distribution<uint64_t> &dist_addr,
                      std::uniform_int_distribution<size_t> &dist_len,
                      std::uniform_int_distribution<int> &dist_byte)
    {
        const uint64_t address = dist_addr(gen);
        size_t length          = dist_len(gen);

        // Clip length to avoid going out of bounds
        if (address + length > TotalMemory) {
            length = TotalMemory - address;
        }

        // Create random data
        std::vector<char> buffer(length);
        for (auto &b : buffer) {
            b = static_cast<char>(dist_byte(gen));
        }

        // Write (we assume SysdarftCPUMemoryAccess is internally thread-safe)
        write_memory(address, buffer.data(), length);
    }

    // Random read helper
    void random_read(std::mt19937 &gen,
                     std::uniform_int_distribution<uint64_t> &dist_addr,
                     std::uniform_int_distribution<size_t> &dist_len)
    {
        const uint64_t address = dist_addr(gen);
        size_t length          = dist_len(gen);

        // Clip length to avoid going out of bounds
        if (address + length > TotalMemory) {
            length = TotalMemory - address;
        }

        // Read (we assume SysdarftCPUMemoryAccess is internally thread-safe)
        std::vector<char> buffer(length);
        read_memory(address, buffer.data(), length);
        // We don't verify buffer content here.
    }
};

int main()
{
    // Instantiate your memory object.
    // (E.g., if SysdarftCPUMemoryAccess has a fixed size of 32MB, it’s already set.)
    Memory memory;

    // Number of threads
    constexpr unsigned NUM_THREADS = 8;
    constexpr unsigned NUM_WRITERS = NUM_THREADS / 2; // e.g., 4
    constexpr unsigned NUM_READERS = NUM_THREADS / 2; // e.g., 4

    // Atomic flag to control the run loop of each thread
    std::atomic<bool> running(true);

    // Writer thread function
    auto writer_func = [&memory, &running](unsigned thread_id)
    {
        // Each thread uses its own RNG engine -> no shared RNG => no data race here
        std::mt19937 gen(std::random_device{}() + thread_id);

        // Distributions
        std::uniform_int_distribution<uint64_t> dist_addr(0, memory.TotalMemory - 1);
        std::uniform_int_distribution<size_t>   dist_len(1, 512);
        std::uniform_int_distribution<int>      dist_byte(0, 255);

        // Loop until 'running' is set to false
        while (running.load(std::memory_order_relaxed)) {
            memory.random_write(gen, dist_addr, dist_len, dist_byte);
        }
    };

    // Reader thread function
    auto reader_func = [&memory, &running](unsigned thread_id)
    {
        std::mt19937 gen(std::random_device{}() + 10000 + thread_id);

        std::uniform_int_distribution<uint64_t> dist_addr(0, memory.TotalMemory - 1);
        std::uniform_int_distribution<size_t>   dist_len(1, 512);

        while (running.load(std::memory_order_relaxed)) {
            memory.random_read(gen, dist_addr, dist_len);
        }
    };

    // We store threads here. Note that we only push_back in the main thread,
    // and only after we finish spawning do we start them concurrently.
    // Then we only join them after we've stopped them.
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    // Spawn writer threads
    for (unsigned i = 0; i < NUM_WRITERS; ++i) {
        threads.emplace_back(writer_func, i);
    }

    // Spawn reader threads
    for (unsigned i = 0; i < NUM_READERS; ++i) {
        threads.emplace_back(reader_func, i);
    }

    // Let the threads run for ~5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Signal all threads to stop
    running.store(false, std::memory_order_relaxed);

    // Join all threads before exiting.
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "All threads finished. If TSan reports no data races here, "
              << "any future TSan warnings likely come from SysdarftCPUMemoryAccess.\n";

    return 0;
}
