#include <res_packer.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

int main()
{
    try {
        initialize_resource_filesystem();
        fuse_start();

        // wait 0.5s for virtual filesystem to set up
        usleep(500000);
        std::ifstream file(RESOURCE_PACK_TMP_DIR "/cmake_modules_Modules.md");

        if (!file) {
            return 1;
        }

        do {
            char in;
            file.get(in);
            std::cout << in;
        } while (file);
        file.close();

        fuse_stop();
        return 0;
    } catch (...) {
        return 1;
    }
}
