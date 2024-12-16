#include <debug.h>
#include <cli.h>
#include <thread>

int main()
{
    Cli screen;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
