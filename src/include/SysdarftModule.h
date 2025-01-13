#ifndef MODULE_H
#define MODULE_H

#include <string>
#include <any>
#include <dlfcn.h>
#include <SysdarftDebug.h>

class SYSDARFT_EXPORT_SYMBOL SysdarftModuleLibraryLoadError final : public SysdarftBaseError {
public:
    explicit SysdarftModuleLibraryLoadError(const std::string & msg) : SysdarftBaseError("Cannot load library: " + msg) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftModuleSymbolResolutionError final : public SysdarftBaseError {
public:
    explicit SysdarftModuleSymbolResolutionError(const std::string & msg) : SysdarftBaseError("Cannot resolve function: " + msg) { }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftModule
{
private:
    std::atomic < void * > handle = nullptr;
    std::mutex ModuleCallMutex;

public:
    template < typename Ret, typename... Args >
    std::any call(const std::string & function_name, const Args &... args)
    {
        std::lock_guard<std::mutex> lock(ModuleCallMutex);
        void *entry = dlsym(handle, function_name.c_str());
        if (const char *error = dlerror()) {
            throw SysdarftModuleSymbolResolutionError(error);
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

    explicit SysdarftModule(const std::string & module_path);
    ~SysdarftModule() { unload(); }
    void init();
    void unload();
    SysdarftModule & operator=(const SysdarftModule &) = delete;
};

#endif //MODULE_H
