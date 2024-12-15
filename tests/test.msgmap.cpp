#include <msg_map.h>
#include <iostream>
#include <thread>
#include <vector>
#include <any>
#include <cassert>

class Instance {
    int c = 0;
public:
    int return20() { return 20; }
    int return_any(const int a) { return a; }
    void registration(const int _c) { c = _c; }
    int getc() { return c; }
};

MsgMap msg_map;
Instance instance;
std::atomic < unsigned int > finished = 0;

// Test Thread1
void Thread1(int off)
{
    for (int i = 0; i < 1000; ++i) {
        int result = std::any_cast<int>(msg_map.invoke_instance("Main", "return_any", {114514}));
        assert(result == 114514); // Fail if result is incorrect
    }

    debug::log("Thread", 3 * off + 1, " done\n");
    ++finished;
}

// Test Thread2
void Thread2(int off)
{
    for (int i = 0; i < 1000; ++i) {
        int result = std::any_cast<int>(msg_map.invoke_instance("Main", "return_any", {1145141919}));
        assert(result == 1145141919); // Fail if result is incorrect
    }

    debug::log("Thread", 3 * off + 2, " done\n");
    ++finished;
}

// Test Thread3
void Thread3(int off)
{
    for (int i = 0; i < 1000; ++i) {
        int result = std::any_cast<int>(msg_map.invoke_instance("Main", "return_any", {123123}));
        assert(result == 123123); // Fail if result is incorrect
    }

    debug::log("Thread", 3 * off + 3, " done\n");
    ++finished;
}

int main()
{
    // Install methods
    msg_map.install_instance("Main", &instance, "return20", &Instance::return20);
    msg_map.install_instance("Main", &instance, "return_any", &Instance::return_any);
    msg_map.install_instance("Main", &instance, "registration", &Instance::registration);
    msg_map.install_instance("Main", &instance, "getc", &Instance::getc);

    // Test the methods with single-threaded calls first
    const auto ret1 = msg_map.invoke_instance("Main", "return20", {});
    assert(std::any_cast<int>(ret1) == 20); // Test for correctness
    const auto ret2 = msg_map.invoke_instance("Main", "return_any", {42});
    assert(std::any_cast<int>(ret2) == 42); // Test for correctness

    // Start multiple threads to test thread safety
    std::vector<std::thread> threads;

    for (int i = 0; i < 100; ++i) {
        threads.emplace_back(Thread1, i);
        threads.emplace_back(Thread2, i);
        threads.emplace_back(Thread3, i);
    }

    // Join all threads
    for (auto& t : threads) {
        t.detach();
    }

    while (finished != 300)
    {
        debug::log("Threads are still alive\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
