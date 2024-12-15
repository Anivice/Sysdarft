#include <debug.h>
#include <cli.h>
#include <thread>

#define INPUT_INSTANCE_NAME "InputProcessor"
#define INPUT_METHOD_NAME "Input"

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

    screen.GlobalEventProcessor.install_instance(INPUT_INSTANCE_NAME, &iprocessor,
        INPUT_METHOD_NAME, &input_processor::input);

    screen.GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &cleanup_handler,
        GLOBAL_DESTROY_METHOD_NAME, &cleanup_handler_::destroy);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
