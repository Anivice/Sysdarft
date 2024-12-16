#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <debug.h>

typedef std::map < std::string /* section */, std::map < std::string /* key */, std::string /* value */> > config_t;

class EXPORT ConfigError final : public SysdarftBaseError {
public:
    ConfigError() : SysdarftBaseError("Config error") { }
};

config_t EXPORT load_config(const std::string & file_name) noexcept(false);

#endif //CONFIG_H
