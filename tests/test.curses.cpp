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
    ui.teletype(0, 1, "Hello, World!");
    sleep(1);
    ui.cleanup();
}
