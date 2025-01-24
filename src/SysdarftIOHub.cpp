/* SysdarftIOHub.cpp
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

#include <mutex>
#include <SysdarftIOHub.h>
#include <SysdarftDebug.h>

SysdarftExternalDeviceBaseClass & SysdarftIOHub::query_device_based_on_port(const uint64_t port)
{
    std::lock_guard list_lock(list_mutex_);
    for (const auto & deviceMap : device_list)
    {
        std::lock_guard buffer_lock(deviceMap->buffer_mutex_);
        for (auto it = deviceMap->device_buffer.begin();
            it != deviceMap->device_buffer.end(); ++it)
        {
            if (port == it->first) {
                return *deviceMap;
            }
        }
    }

    throw SysdarftNoSuchDevice("Device providing IO port " + std::to_string(port) + " not found!");
}

ControllerDataStream & SysdarftIOHub::ins(const uint64_t port)
{
    auto & device = query_device_based_on_port(port);
    if (!device.request_read(port)) {
        throw SysdarftDeviceIOError("Input error on port " + std::to_string(port));
    }

    std::lock_guard lock(device.buffer_mutex_);
    return *device.device_buffer.at(port);
}

void SysdarftIOHub::outs(const uint64_t port, ControllerDataStream & buffer)
{
    auto && device = query_device_based_on_port(port);
    std::lock_guard lock(device.buffer_mutex_);
    device.device_buffer.at(port)->insert(buffer);

    if (!device.request_write(port)) {
        throw SysdarftDeviceIOError("Output error on port " + std::to_string(port));
    }
}
