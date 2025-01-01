#include <MessageMap.h>

std::any MessageMap::invoke_instance(const std::string& instance_name, const std::string& method_name, const std::vector<std::any>& args)
{
    std::lock_guard lock(mutex);

    const std::string key = instance_name + "::" + method_name;
    const auto it = methods.find(key);
    if (it == methods.end()) {
        throw MessageMapNoSuchInstance();
    }
    return it->second(args);
}

MessageMap::wrapper MessageMap::operator()(const std::string& instance_name, const std::string& method_name)
{
    std::lock_guard lock(mutex);

    const std::string key = instance_name + "::" + method_name;
    const auto it = methods.find(key);
    if (it == methods.end()) {
        throw MessageMapNoSuchInstance();
    }

    return wrapper(it->second);;
}
