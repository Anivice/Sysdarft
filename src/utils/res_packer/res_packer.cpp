#include <algorithm>
#include <debug.h>
#include <res_packer.h>
#include <resource_file_list.h>
#include <thread>
#include <map>

void fuse_start();
void fuse_stop();
std::map < std::string, resource_file_t > resource_file_folder;

std::string process_path_name(std::string pathname)
{
    std::ranges::replace(pathname, '/', '_');
    return pathname;
}

void initialize_resource_filesystem()
{
    __initialization_for_resource_file_vectors__();

    unsigned int index = 0;
    for (; index < resource_file_count; index++)
    {
        std::string filename = resource_file_list[index];
        resource_file_t file = {
            .file_length = resource_file_content_length_vector[index],
            .file_content = resource_file_content_vector[index]
        };

        resource_file_folder.emplace(filename, file);
    }
}

[[nodiscard]] const std::map < std::string, resource_file_t > & get_res_file_list() noexcept {
    return resource_file_folder;
}
