#include <module.h>

Module::Module(const std::string & module_path)
{
    handle = dlopen(module_path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw LibraryLoadError(dlerror());
    }
}

void Module::unload()
{
    if (handle)
    {
        call<void>("module_exit");
        dlclose(handle);
        handle = nullptr;
    }
}

void Module::init()
{
    if (std::any_cast<int>(call<int>("module_init")) != 0) {
        throw LibraryLoadError("Module initialization failed");
    }
}

void Module::close_only()
{
    if (handle)
    {
        dlclose(handle);
        handle = nullptr;
    }
}
