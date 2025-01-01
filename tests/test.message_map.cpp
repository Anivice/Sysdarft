#include <iostream>
#include <thread>
#include <vector>
#include <any>
#include <cassert>
#include <MessageMap.h>
#include <SysdarftDebug.h>

class Instance {
public:
    std::atomic < int > c = 0;
    int return20() { return 20; }
    int return_any(const int a) { return a; }
    void registration_add_one() {
        c.fetch_add(1, std::memory_order_relaxed); // or memory_order_seq_cst
    }
};

MessageMap msg_map;
Instance instance;
std::atomic < unsigned int > finished = 0;

// Test Thread1
void Thread1(int off)
{
    for (int i = 0; i < 1000; ++i) {
        const int result = std::any_cast<int>(msg_map.invoke_instance("Main", "return_any", {114514}));
        assert(result == 114514); // Fail if result is incorrect
    }

    log("Thread", 3 * off + 1, " done\n");
    ++finished;
}

// Test Thread2
void Thread2(int off)
{
    for (int i = 0; i < 1000; ++i) {
        msg_map("Main", "registration")();
    }

    log("Thread", 3 * off + 2, " done\n");
    ++finished;
}

// Test Thread3
void Thread3(int off)
{
    for (int i = 0; i < 1000; ++i) {
        msg_map("Main", "registration")();
    }

    log("Thread", 3 * off + 3, " done\n");
    ++finished;
}

int main()
{
    // Install methods
    msg_map.install_instance("Main", &instance, "return20", &Instance::return20);
    msg_map.install_instance("Main", &instance, "return_any", &Instance::return_any);
    msg_map.install_instance("Main", &instance, "registration", &Instance::registration_add_one);

    // Test the methods with single-threaded calls first
    const auto ret1 = msg_map.invoke_instance("Main", "return20", {});
    assert(std::any_cast<int>(ret1) == 20); // Test for correctness
    const auto ret2 = msg_map.invoke_instance("Main", "return_any", {42});
    assert(std::any_cast<int>(ret2) == 42); // Test for correctness

    // Start multiple threads to test thread safety
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(Thread1, i);
        threads.emplace_back(Thread2, i);
        threads.emplace_back(Thread3, i);
    }

    // Join all threads
    for (auto& t : threads)
    {
        if (t.joinable()) {
            t.join();
        }
    }

    while (finished != 30)
    {
        log("Threads are still alive\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (instance.c != 20000) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
