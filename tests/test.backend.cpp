#include <GlobalEvents.h>
#include <SysdarftModule.h>
#include <thread>

int main()
{
    SysdarftModule GUI("./gui_module/libsysdarft_backend.so");
    g_ui_initialize();
    g_ui_initialize();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    g_ui_cleanup();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    g_ui_initialize();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    g_ui_cleanup();
}
