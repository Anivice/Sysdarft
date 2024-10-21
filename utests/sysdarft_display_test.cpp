#include <iostream>
#include <sysdarft_display.h>

int main()
{
    try {
        // sysdarft_display.set_cursors_visibility(true);
        sysdarft_display.display_char(2, 0, '_');
        sysdarft_display.display_char(2, 1, '>');
        sysdarft_display.display_char(2, 2, 'J');
        sysdarft_display.display_char(2, 3, 'u');
        sysdarft_display.display_char(2, 4, 'e');
        sysdarft_display.display_char(2, 5, 'C');
        sysdarft_display.display_char(2, 6, 'h');
        sysdarft_display.display_char(2, 7, 'e');
        sysdarft_display.display_char(2, 8, 'n');
        sysdarft_display.display_char(2, 9, 'g');
        sysdarft_display.display_char(2, 10, ' ');
        sysdarft_display.display_char(2, 11, 'I');
        sysdarft_display.display_char(2, 12, ' ');
        sysdarft_display.display_char(2, 13, 'l');
        sysdarft_display.display_char(2, 14, 'o');
        sysdarft_display.display_char(2, 15, 'v');
        sysdarft_display.display_char(2, 16, 'e');
        sysdarft_display.display_char(2, 17, ' ');
        sysdarft_display.display_char(2, 18, 'y');
        sysdarft_display.display_char(2, 19, 'o');
        sysdarft_display.display_char(2, 20, 'u');
        sysdarft_display.display_char(2, 21, '!');
        std::cout << sysdarft_display.get_char_at_pos(2, 0) << std::endl;
        sysdarft_display.sleep(3);
        return 0;
    } catch (py::error_already_set & err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
