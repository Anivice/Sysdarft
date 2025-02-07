/* SysdarftDebug.inl
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef DEBUG_INL
#define DEBUG_INL

#include <iostream>

template <typename... Strings>
debug::cmd_status debug::exec_command(const std::string &cmd, const std::string &input,
                                      Strings &&...args)
{
    const std::vector<std::string> vec{std::forward<Strings>(args)...};
    const auto csysroot = std::getenv("SYSROOT");
    std::string sysroot;
    if (csysroot != nullptr) {
        sysroot = csysroot;
    }

    if (debug::verbose) {
        d_log("Executing ", sysroot, "/", cmd, " ", vec, "\n");
        const auto ret = _exec_command(sysroot + "/" + cmd, vec, input);
        d_log("Exit status for executed command: ", ret.exit_status, ", stderr: ", ret.fd_stderr, "\n");
        return ret;
    }

    return _exec_command(sysroot + "/" + cmd, vec, input);
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
    setvbuf(stderr, nullptr, _IONBF, 0);
    std::lock_guard lock(log_mutex);
    debug::_log(args...);
    std::cerr << std::flush << std::flush;
    fflush(stderr);
}

#endif // DEBUG_INL
