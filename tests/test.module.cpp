#include <thread>
#include <SysdarftModule.h>

int main()
{
    SysdarftModule Module("./libExampleModule.so");

    auto caller = [&Module]() {
        for (int i = 0; i < 100; i++) {
            Module.call<void>("inc");
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 100; i++) {
        threads.emplace_back(caller);
    }

    for (auto &thread : threads)
    {
        if (thread.joinable()) {
            thread.join();
        }
    }

    int result = std::any_cast<int>(Module.call<int>("get"));
    if (result != 10000) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
