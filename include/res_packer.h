#ifndef RES_PACKER_H
#define RES_PACKER_H

#include <map>
#include <string>

struct resource_file_t {
    const unsigned int file_length;
    const unsigned char * file_content;
};

void initialize_resource_filesystem();
void fuse_start();
void fuse_stop();

[[nodiscard]] const std::map < std::string, resource_file_t > & get_res_file_list() noexcept;

#endif //RES_PACKER_H
