/* SysdarftModule.h
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
