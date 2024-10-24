#include <iostream>
#include <sysdarft_display.h>
#include <thread>

int display()
{
    try {
        sysdarft_display_t gpu;
        gpu.initialize();
        gpu.display_char(2, 0, '_');
        gpu.display_char(2, 1, '>');
        gpu.display_char(2, 2, 'J');
        gpu.display_char(2, 3, 'u');
        gpu.display_char(2, 4, 'e');
        gpu.display_char(2, 5, 'C');
        gpu.display_char(2, 6, 'h');
        gpu.display_char(2, 7, 'e');
        gpu.display_char(2, 8, 'n');
        gpu.display_char(2, 9, 'g');
        gpu.display_char(2, 10, ' ');
        gpu.display_char(2, 11, 'I');
        gpu.display_char(2, 12, ' ');
        gpu.display_char(2, 13, 'l');
        gpu.display_char(2, 14, 'o');
        gpu.display_char(2, 15, 'v');
        gpu.display_char(2, 16, 'e');
        gpu.display_char(2, 17, ' ');
        gpu.display_char(2, 18, 'y');
        gpu.display_char(2, 19, 'o');
        gpu.display_char(2, 20, 'u');
        gpu.display_char(2, 21, '!');
        gpu.unblocked_sleep(3000);
        gpu.cleanup();
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
