#include <cpu.h>

int main()
{
    // sysdarft_collaboration();
    sysdarft_start();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    sysdarft_stop();
}
