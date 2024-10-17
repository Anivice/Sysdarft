#include <sysdarft_display.h>
#include <iostream>

int main()
{
    try {
        sysdarft_display.display_char(0, 0, 'A');
        sysdarft_display.display_char(0, 1, 'B');
        sysdarft_display.sleep(3);
        return 0;
    } catch (py::error_already_set & err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
