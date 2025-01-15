#include <csignal>
#include <SysdarftCursesUI.h>

// Global pointer to the UI instance (for signal handler access)
SysdarftCursesUI* g_ui_instance = nullptr;

// Signal handler for window resize
void resize_handler(int sig) {
    if (g_ui_instance) {
        g_ui_instance->handle_resize();
    }
}

int main()
{
    SysdarftCursesUI ui;
    g_ui_instance = &ui;

    // Register the SIGWINCH handler
    std::signal(SIGWINCH, resize_handler);

    ui.initialize();
    ui.teletype('S');
    sleep(1);
    ui.cleanup();

    sleep(1);

    ui.start_again();
    sleep(1);
    ui.cleanup();

    ui.ringbell();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ui.ringbell();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ui.ringbell();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
