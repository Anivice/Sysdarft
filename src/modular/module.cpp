#include <module.h>

Module::Module(const std::string & module_path)
{
    handle = dlopen(module_path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw LibraryLoadError(dlerror());
    }

    if (std::any_cast<int>(call<int>("module_init")) != 0) {
        throw LibraryLoadError("Module initialization failed");
    }
}

Module::~Module()
{
    unload();
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
