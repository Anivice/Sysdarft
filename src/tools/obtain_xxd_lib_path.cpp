#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <debug.h>

std::vector<std::string> & remove_duplicates(std::vector<std::string>& vec)
{
    // Sort the vector
    std::sort(vec.begin(), vec.end());

    // Use std::unique to move duplicates to the end
    auto last = std::unique(vec.begin(), vec.end());

    // Erase the duplicates
    vec.erase(last, vec.end());

    return vec;
}

std::vector < std::string > list_shared_libraries()
{
    std::ifstream maps_file("/proc/self/maps");
    std::string line;
    std::vector < std::string > ret;

    if (!maps_file.is_open()) {
        log(sysdarft_log::LOG_ERROR, "[TOOLS] Failed to open /proc/self/maps\n");
        throw sysdarft_error_t(sysdarft_error_t::CANNOT_OBTAIN_DYNAMIC_LIBRARIES);
    }

    while (std::getline(maps_file, line))
    {
        // Look for lines that indicate shared libraries.
        // These lines contain the string ".so"
        if (line.find(".so") != std::string::npos)
        {
            const std::size_t path_start = line.find('/');
            if (path_start != std::string::npos)
            {
                std::string lib_path = line.substr(path_start);
                ret.emplace_back(lib_path);
            }
        }
    }

    remove_duplicates(ret);
    return ret;
}

std::vector<std::string> find_containing_substring(const std::vector<std::string>& vec, const std::string& target)
{
    std::vector<std::string> result;

    for (const auto& str : vec) {
        // Check if the string contains the target as a substring but is not equal to it
        if (str.find(target) != std::string::npos && str != target) {
            result.push_back(str);  // Add to the result if the condition is met
        }
    }

    return result;
}
