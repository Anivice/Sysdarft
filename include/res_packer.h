#ifndef RES_PACKER_H
#define RES_PACKER_H

#include <map>
#include <string>

extern class res_packer_t
{
private:
    struct resource_file_t {
        unsigned int file_length;
        const unsigned char * file_content;
    };

    std::map < std::string /* file path name, like scripts_AmberScreenEmulator.py */,
            resource_file_t > resource_file_folder;

public:
    res_packer_t();

    res_packer_t & operator=(const res_packer_t &) = delete;
} res_packer;

#endif //RES_PACKER_H
