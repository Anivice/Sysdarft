#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <atomic>
#include <chrono>

// Include your SysdarftMemoryAccess header
#include <SysdarftMemory.h>

// A simple derived class that just uses the SysdarftCPUMemoryAccess
class Memory : public SysdarftCPUMemoryAccess
{
public:
    Memory() = default;

    const uint64_t TotalMemory = SysdarftCPUMemoryAccess::TotalMemory;

    // Helper function for a random write
    void random_write(std::mt19937 &gen,
                      std::uniform_int_distribution<uint64_t> &dist_addr,
                      std::uniform_int_distribution<size_t> &dist_len,
                      std::uniform_int_distribution<int> &dist_byte)
    {
        // Pick a random address and length
        uint64_t address = dist_addr(gen);
        size_t   length  = dist_len(gen);

        // Ensure we don't go out of bounds
        if (address + length > TotalMemory) {
            length = TotalMemory - address;
        }

        // Generate random data
        std::vector<char> buffer(length);
        for (auto &b : buffer) {
            b = static_cast<char>(dist_byte(gen));
        }

        // Write to memory
        write_memory(address, buffer.data(), length);
    }

    // Helper function for a random read
    void random_read(std::mt19937 &gen,
                     std::uniform_int_distribution<uint64_t> &dist_addr,
                     std::uniform_int_distribution<size_t> &dist_len)
    {
        // Pick a random address and length
        uint64_t address = dist_addr(gen);
        size_t   length  = dist_len(gen);

        // Ensure we don't go out of bounds
        if (address + length > TotalMemory) {
            length = TotalMemory - address;
        }

        // Read from memory
        std::vector<char> buffer(length);
        read_memory(address, buffer.data(), length);

        // We do NOT verify what's in 'buffer' here
        // because other threads may have overwritten it.
        // This test is purely to ensure no crashes, no corruption,
        // and correct bounds/mutex usage.
    }
};

int main()
{
    // Create the memory object (32 MB or whatever you have in SysdarftMemory)
    Memory memory;

    // We will run multiple threads: half do random writes, half do random reads.
    // Or you can do a 1:1 ratio, etc.
    const unsigned NUM_THREADS = 8;
    const unsigned NUM_WRITERS = NUM_THREADS / 2; // e.g., 4
    const unsigned NUM_READERS = NUM_THREADS / 2; // e.g., 4

    // Each thread will run until 'running' becomes false
    std::atomic<bool> running(true);

    // Writer thread function
    auto writer_func = [&memory, &running](unsigned thread_id)
    {
        // Each thread uses its own RNG to avoid data races in the PRNG
        std::mt19937 gen(std::random_device{}() + thread_id);
        std::uniform_int_distribution<uint64_t> dist_addr(0, memory.TotalMemory - 1);
        std::uniform_int_distribution<size_t>   dist_len(1, 512); // can tweak max length
        std::uniform_int_distribution<int>      dist_byte(0, 255);

        while (running.load(std::memory_order_relaxed)) {
            memory.random_write(gen, dist_addr, dist_len, dist_byte);
        }
    };

    // Reader thread function
    auto reader_func = [&memory, &running](unsigned thread_id)
    {
        // Each thread uses its own RNG
        // Add 10000 to separate seeds from writers if you like
        std::mt19937 gen(std::random_device{}() + 10000 + thread_id);
        std::uniform_int_distribution<uint64_t> dist_addr(0, memory.TotalMemory - 1);
        std::uniform_int_distribution<size_t>   dist_len(1, 512);

        while (running.load(std::memory_order_relaxed)) {
            memory.random_read(gen, dist_addr, dist_len);
        }
    };

    // Create threads
    std::vector<std::thread> pool;
    pool.reserve(NUM_THREADS);

    // Launch writer threads
    for (unsigned i = 0; i < NUM_WRITERS; i++) {
        pool.emplace_back(writer_func, i);
    }
    // Launch reader threads
    for (unsigned i = 0; i < NUM_READERS; i++) {
        pool.emplace_back(reader_func, i);
    }

    // Let them run for ~5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Signal all threads to stop
    running.store(false, std::memory_order_relaxed);

    // Join
    for (auto &th : pool) {
        th.join();
    }

    std::cout << "All threads finished. If the program did not crash "
              << "or throw exceptions, the read/write is likely thread-safe.\n";

    return 0;
}
