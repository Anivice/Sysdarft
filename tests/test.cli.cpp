#include <debug.h>
#include <cli.h>
#include <thread>

class input_processor {
public:
    void input(const std::vector<std::string> & args)
    {
        debug::log(args, '\n');
    }
} iprocessor;

class cleanup_handler_ {
public:
    void destroy()
    {
        debug::log("Requesting termination...\n");
        exit(EXIT_SUCCESS);
    }
} cleanup_handler;

int main()
{
    Cli screen;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
