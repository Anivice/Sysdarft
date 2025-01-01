#include <SysdarftModule.h>

SysdarftModule::SysdarftModule(const std::string & module_path)
{
    handle = dlopen(module_path.c_str(), RTLD_LAZY);
    if (!handle) {
        throw SysdarftModuleLibraryLoadError(dlerror());
    }

    try {
        init();
    } catch (const std::exception &) {
        dlclose(handle);
        throw;
    }
}

void SysdarftModule::unload()
{
    if (handle)
    {
        call<void>("module_exit");
        dlclose(handle);
        handle = nullptr;
    }
}

void SysdarftModule::init()
{
    if (std::any_cast<int>(call<int>("module_init")) != 0) {
        throw SysdarftModuleLibraryLoadError("Module initialization failed");
    }
}
