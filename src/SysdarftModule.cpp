/* SysdarftModule.cpp
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
