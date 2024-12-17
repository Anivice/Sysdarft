#include <debug.h>
#include <cli.h>
#include <thread>

int main()
{
    debug::verbose = true;
    Cli screen;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    GlobalEventProcessor(GLOBAL_INSTANCE_NAME, GLOBAL_DESTROY_METHOD_NAME)();
    while (GlobalEventNotifier != GLOBAL_QUIT_EVENT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    GlobalEventNotifier = 0;
    return EXIT_SUCCESS;
}
