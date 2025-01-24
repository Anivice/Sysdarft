/* SysdarftMessageMap.cpp
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

#include <SysdarftMessageMap.h>

std::any SysdarftMessageMap::invoke_instance(const std::string& instance_name, const std::string& method_name, const std::vector<std::any>& args)
{
    std::lock_guard lock(mutex);

    const std::string key = instance_name + "::" + method_name;
    const auto it = methods.find(key);
    if (it == methods.end()) {
        throw MessageMapNoSuchInstance();
    }
    return it->second(args);
}

SysdarftMessageMap::wrapper SysdarftMessageMap::operator()(const std::string& instance_name, const std::string& method_name)
{
    std::lock_guard lock(mutex);

    const std::string key = instance_name + "::" + method_name;
    const auto it = methods.find(key);
    if (it == methods.end()) {
        throw MessageMapNoSuchInstance();
    }

    return wrapper(it->second);;
}
