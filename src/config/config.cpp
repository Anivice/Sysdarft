#include <config.h>
#include <fstream>

config_t load_config(const std::string & file_name)
{
    std::ifstream file(file_name);

    if (!file.is_open()) {
        throw ConfigError();
    }

    auto clean = [&](std::string & str) {
        std::erase(str, ' ');
    };

    try
    {
        config_t config;
        std::string line;
        std::string section;

        while (std::getline(file, line))
        {
            clean(line);

            // Skip empty lines and comments
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;

            if (line[0] == '[') {
                // Parse section
                section = line.substr(1, line.find(']') - 1);
            } else {
                // Parse key=value
                if (size_t delimiterPos = line.find('='); delimiterPos != std::string::npos)
                {
                    std::string key = line.substr(0, delimiterPos);
                    std::string value = line.substr(delimiterPos + 1);
                    config[section][key] = value;
                }
            }
        }

        // sanity check:
        for (const auto&[fst, snd] : config)
        {
            if (fst.empty()) {
                throw ConfigError();
            }

            for (const auto&[key, val] : snd)
            if (key.empty() || val.empty()) {
                throw ConfigError();
            }
        }

        return config;
    } catch (...) {
        throw ConfigError();
    }
}
