/* SysdarftMessageMap.h
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

#ifndef MSG_MAP_H
#define MSG_MAP_H

#include <map>
#include <string>
#include <functional>
#include <any>
#include <utility>
#include <vector>
#include <tuple>
#include <type_traits>
#include <mutex>
#include <SysdarftDebug.h>

class MessageMapNoSuchInstance final : public SysdarftBaseError
{
public:
    MessageMapNoSuchInstance() : SysdarftBaseError("No such instance") { }
};

class MessageMapArgumentMismatch final : public SysdarftBaseError
{
public:
    MessageMapArgumentMismatch() : SysdarftBaseError("Argument count mismatch") { }
};

// Forward declaration of the helper
template <typename... Args, std::size_t... I>
std::tuple<Args...> any_cast_tuple_impl(const std::vector<std::any>& args, std::index_sequence<I...>);

// Primary template function
template <typename... Args>
std::tuple<Args...> any_cast_tuple(const std::vector<std::any>& args)
{
    if (args.size() != sizeof...(Args)) {
        throw MessageMapArgumentMismatch();
    }

    return any_cast_tuple_impl<Args...>(args, std::make_index_sequence<sizeof...(Args)>{});
}

template <typename... Args, std::size_t... I>
std::tuple<Args...> any_cast_tuple_impl(const std::vector<std::any>& args, std::index_sequence<I...>)
{
    return std::tuple<Args...>(std::any_cast<Args>(args[I])...);
}

template <typename Func, typename... Args>
std::any invoke_with_any(Func func, const std::vector<std::any>& args)
{
    auto tuple_args = any_cast_tuple<Args...>(args);
    if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
        std::apply(func, tuple_args);
        return std::any{};
    } else {
        return std::apply(func, tuple_args);
    }
}

class SYSDARFT_EXPORT_SYMBOL SysdarftMessageMap
{
private:
    class wrapper
    {
    private:
        std::function<std::any(const std::vector<std::any>&)> method_;
        explicit wrapper(decltype(method_)  _method) : method_(std::move(_method)) { }

    public:
        template <typename... Args>
        std::any operator()(const Args&... args)
        {
            const std::vector<std::any> vargs = { args... };
            return method_(vargs);
        }

        wrapper & operator=(const wrapper&) = delete;
        friend class SysdarftMessageMap;
    };

public:
    template <typename InstanceType, typename ReturnType, typename... Args>
    void install_instance(
        const std::string& instance_name,
        InstanceType* instance,
        const std::string& method_name,
        ReturnType (InstanceType::*method)(Args...))
    {
        std::lock_guard lock(mutex);

        // Bind the method to the instance
        auto bound_function = [instance, method](Args... args) -> ReturnType {
            return (instance->*method)(args...);
        };

        // Create a key for the map
        const std::string key = instance_name + "::" + method_name;

        // Store a lambda that accepts vector<any> and invokes the bound_function
        methods[key] = [bound_function](const std::vector<std::any>& args) -> std::any {
            return invoke_with_any<decltype(bound_function), Args...>(bound_function, args);
        };
    }

    std::any invoke_instance(const std::string& instance_name, const std::string& method_name, const std::vector<std::any>& args);
    wrapper operator()(const std::string& instance_name, const std::string& method_name);

private:
    std::map<std::string, std::function<std::any(const std::vector<std::any>&)>> methods;
    std::mutex mutex;
};

#endif //MSG_MAP_H
