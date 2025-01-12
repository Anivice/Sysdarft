#ifndef DEBUG_INL
#define DEBUG_INL

#include <iostream>

template <typename... Strings>
debug::cmd_status debug::exec_command(const std::string &cmd, const std::string &input,
                                      Strings &&...args)
{
    const std::vector<std::string> vec{std::forward<Strings>(args)...};
    return _exec_command(cmd, vec, input);
}

template <typename Container>
std::enable_if_t < debug::is_container_v<Container>
    && !debug::is_map_v<Container>
    && !debug::is_unordered_map_v<Container>
, void >
debug::print_container(const Container &container)
{
    std::cerr << "[";
    for (auto it = std::begin(container); it != std::end(container); ++it)
    {
        std::cerr << *it;
        if (std::next(it) != std::end(container)) {
            std::cerr << ", ";
        }
    }
    std::cerr << "]";
}

template <typename Map>
std::enable_if_t < debug::is_map_v<Map> || debug::is_unordered_map_v<Map>, void >
debug::print_container(const Map &map)
{
    std::cerr << "{";
    for (auto it = std::begin(map); it != std::end(map); ++it)
    {
        std::cerr << it->first << ": " << it->second;
        if (std::next(it) != std::end(map)) {
            std::cerr << ", ";
        }
    }
    std::cerr << "}";
}

template <typename ParamType> void debug::_log(const ParamType &param)
{
    if constexpr (debug::is_string_v<ParamType>) {
        // Handle std::string and const char*
        std::cerr << param;
    } else if constexpr (debug::is_container_v<ParamType>) {
        // Handle containers
        debug::print_container(param);
    }
    else {
        // Handle other types
        std::cerr << param;
    }
}

template <typename ParamType, typename... Args>
void debug::_log(const ParamType &param, const Args &...args)
{
    debug::_log(param);
    (debug::_log(args), ...);
}

template <typename... Args> void debug::d_log(const Args &...args)
{
    std::lock_guard lock(log_mutex);
    debug::_log(args...);
}

#endif // DEBUG_INL
