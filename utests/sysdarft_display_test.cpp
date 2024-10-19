#include <iostream>
#include <res_packer.h>
#include <sysdarft_display.h>

#include "debug.h"

int main()
{
    try {
        // initialize_resource_filesystem();
        // sysdarft_display.initialize();

        sysdarft_display.display_char(0, 0, 'A');
        // sysdarft_display.display_char(0, 1, 'B');
        sysdarft_display.sleep(1);

        // sysdarft_display.cleanup();
        return 0;
    } catch (py::error_already_set & err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
