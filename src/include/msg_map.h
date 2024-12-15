#ifndef MSG_MAP_H
#define MSG_MAP_H

#include <debug.h>
#include <map>
#include <string>
#include <functional>
#include <any>
#include <utility>
#include <vector>
#include <tuple>
#include <type_traits>
#include <mutex>

class NoSuchInstance final : public SysdarftBaseError
{
public:
    NoSuchInstance() : SysdarftBaseError("No such instance") { }
};

class ArgumentMismatch final : public SysdarftBaseError
{
public:
    ArgumentMismatch() : SysdarftBaseError("Argument count mismatch") { }
};

// Helper function to cast a vector of std::any to a tuple of arguments
template <typename... Args>
std::tuple<Args...> any_cast_tuple(const std::vector<std::any>& args)
{
    if (args.size() != sizeof...(Args)) {
        throw ArgumentMismatch();
    }

    // Use a lambda to convert each element of args to the corresponding type
    return std::tuple<Args...>(std::any_cast<Args>(args[0])...); // expanded with the number of Args
}

template <typename Func, typename... Args>
std::any invoke_with_any(Func func, const std::vector<std::any>& args)
{
    // Convert std::vector<std::any> to std::tuple<Args...>
    auto tuple_args = any_cast_tuple<Args...>(args);
    // Call std::apply to invoke func with the unpacked tuple
    if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...>>) {
        std::apply(func, tuple_args);
        return std::any{};
    } else {
        return std::apply(func, tuple_args);
    }
}

class MsgMap
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
        friend class MsgMap;
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

    std::any invoke_instance(const std::string& instance_name, const std::string& method_name, const std::vector<std::any>& args)
    {
        std::lock_guard lock(mutex);

        const std::string key = instance_name + "::" + method_name;
        const auto it = methods.find(key);
        if (it == methods.end()) {
            throw NoSuchInstance();
        }
        return it->second(args);
    }

    wrapper operator()(const std::string& instance_name, const std::string& method_name)
    {
        std::lock_guard lock(mutex);

        const std::string key = instance_name + "::" + method_name;
        const auto it = methods.find(key);
        if (it == methods.end()) {
            throw NoSuchInstance();
        }

        return wrapper(it->second);;
    }

private:
    std::map<std::string, std::function<std::any(const std::vector<std::any>&)>> methods;
    std::mutex mutex;
};

#endif //MSG_MAP_H
