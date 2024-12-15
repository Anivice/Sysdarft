#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <any>
#include <dlfcn.h>
#include <debug.h>

class LibraryLoadError final : public SysdarftBaseError {
public:
    explicit LibraryLoadError(const std::string & msg) : SysdarftBaseError("Cannot load library: " + msg) { }
};

class ModuleResolutionError final : public SysdarftBaseError {
public:
    explicit ModuleResolutionError(const std::string & msg) : SysdarftBaseError("Cannot resolve function: " + msg) { }
};

class Module
{
private:
    void * handle = nullptr;

public:
    Module() = default;
    Module & operator=(const Module &) = default;

    explicit Module(const std::string & module_path);

    template < typename Ret, typename... Args >
    std::any call(const std::string & function_name, const Args &... args)
    {
        void *entry = dlsym(handle, function_name.c_str());
        if (const char *error = dlerror()) {
            throw ModuleResolutionError(error);
        }

        using function_type = Ret(*)(Args...);
        auto func = reinterpret_cast<function_type>(entry);

        if constexpr (std::is_same_v<Ret, void>)
        {
            // If Ret is void, call the function and return nothing
            func(args...);
            return {};
        } else {
            Ret result = func(args...);
            return std::any(result);
        }
    }

    void unload();
};

#endif //MODULE_H
