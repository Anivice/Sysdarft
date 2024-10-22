#include <iostream>
#include <sysdarft_display.h>
#include <thread>

int display()
{
    try {
        sysdarft_gpu_t gpu;
        gpu.initialize();
        auto local_vm = gpu.video_memory.load();
        local_vm[2][0] = '_';
        local_vm[2][1] = '>';
        local_vm[2][2] = 'J';
        local_vm[2][3] = 'u';
        local_vm[2][4] = 'e';
        local_vm[2][5] = 'C';
        local_vm[2][6] = 'h';
        local_vm[2][7] = 'e';
        local_vm[2][8] = 'n';
        local_vm[2][9] = 'g';
        local_vm[2][10] = ' ';
        local_vm[2][11] = 'I';
        local_vm[2][12] = ' ';
        local_vm[2][13] = 'l';
        local_vm[2][14] = 'o';
        local_vm[2][15] = 'v';
        local_vm[2][16] = 'e';
        local_vm[2][17] = ' ';
        local_vm[2][18] = 'y';
        local_vm[2][19] = 'o';
        local_vm[2][20] = 'u';
        local_vm[2][21] = '!';
        gpu.video_memory.store(local_vm);
        gpu.sleep_without_blocking(3000);
        return 0;
    } catch (py::error_already_set & err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}

int main()
{
    // display();
    std::thread Thread(display);
    Thread.join();
}
