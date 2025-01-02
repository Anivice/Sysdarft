#include <SysdarftCursesUI.h>
#include <atomic>

std::atomic<bool> has_q = false;

class dummy_ {
public:
    void input_processor(int input) {
        if (input == 'q' || input == 'Q') {
            has_q = true;
        }
    }
} dummy;

int main(int argc, char **)
{
    SysdarftCursesUI curses;
    g_input_processor_install(dummy, input_processor);

    auto worker = [&]() {
        curses.initialize();
        if (argc >= 2) {
            while (!has_q) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        curses.cleanup();
    };

    for (int i = 0; i < 10; i++) {
        worker();
    }

    return EXIT_SUCCESS;
}
