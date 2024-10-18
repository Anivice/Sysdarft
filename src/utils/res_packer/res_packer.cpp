#include <res_packer.h>
#include <algorithm>
#include <resource_file_list.h>
#include <debug.h>

res_packer_t res_packer;

inline std::string process_path_name(std::string pathname)
{
    std::ranges::replace(pathname, '/', '_');
    return pathname;
}

res_packer_t::res_packer_t()
{
    constexpr unsigned int resource_file_count = sizeof(resource_file_content_length_vector) / sizeof(unsigned int);
    unsigned int index = 0;
    for (; index < resource_file_count; index++)
    {
        std::string filename = process_path_name(resource_file_list[index]);
        resource_file_t file = {
            .file_length = resource_file_content_length_vector[index],
            .file_content = resource_file_content_vector[index]
        };

        resource_file_folder.emplace(filename, file);
        sysdarft_log::log(sysdarft_log::LOG_NORMAL, sysdarft_log::GREEN,
            "Resource file (#", index, ") ", filename, " loaded. File size = ",
                resource_file_content_length_vector[index], "\n",
            sysdarft_log::REGULAR);
    }
}
