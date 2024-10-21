#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

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
        std::cerr << "Failed to open /proc/self/maps" << std::endl;
        return ret;
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

int main()
{
    for (auto file : list_shared_libraries()) {
        std::cout << file << std::endl;
    }
    return 0;
}
