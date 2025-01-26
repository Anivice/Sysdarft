/* SysdarftIOHub.h
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

#ifndef SYSDARFTIOHUB_H
#define SYSDARFTIOHUB_H

#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <SysdarftDebug.h>

class SysdarftNoSuchDevice final : public SysdarftBaseError
{
public:
    explicit SysdarftNoSuchDevice(const std::string& msg) : SysdarftBaseError("No such device: " + msg) { }
};

class SysdarftDeviceIOError : public SysdarftBaseError {
public:
    explicit SysdarftDeviceIOError(const std::string& msg) : SysdarftBaseError("Device I/O Error: " + msg) { }
};

class ControllerDataStream {
private:
    std::mutex buffer_mutex_;
    std::vector < uint8_t > device_buffer;

public:
    template < typename DataType >
    void push(const DataType & data)
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        for (unsigned int i = 0; i < sizeof(data); i++) {
            device_buffer.emplace_back(((uint8_t*)&data)[i]);
        }
    }

    template < typename DataType >
    DataType pop()
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        DataType data { };
        for (unsigned int i = 0; i < sizeof(data); i++)
        {
            if (device_buffer.empty()) {
                throw SysdarftDeviceIOError("Device buffer is empty");
            } else {
                ((uint8_t*)&data)[i] = device_buffer.front();
                device_buffer.erase(device_buffer.begin());
            }
        }

        return data;
    }

    void insert(const std::vector<uint8_t> & data)
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        device_buffer.insert(device_buffer.end(), data.begin(), data.end());
    }

    void insert(ControllerDataStream & data)
    {
        std::lock_guard lock(buffer_mutex_);
        std::lock_guard lock2(data.buffer_mutex_);
        device_buffer.insert(device_buffer.end(), data.device_buffer.begin(), data.device_buffer.end());
    }

    std::vector < uint8_t > getObject()
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        return device_buffer;
    }

    [[nodiscard]] uint64_t getSize()
    {
        std::lock_guard lock(buffer_mutex_);
        return device_buffer.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        device_buffer.clear();
    }

    ControllerDataStream & operator=(ControllerDataStream&) = delete;
};

class SysdarftExternalDeviceBaseClass
{
public:
    virtual ~SysdarftExternalDeviceBaseClass() = default;
    std::mutex buffer_mutex_;
    std::map < uint64_t /* IO Port */, std::unique_ptr < ControllerDataStream > > device_buffer;
    virtual bool request_read(uint64_t /* IO Port */) { return false; }
    virtual bool request_write(uint64_t /* IO Port */) { return false; }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftIOHub
{
private:
    SysdarftExternalDeviceBaseClass & query_device_based_on_port(uint64_t);

protected:
    std::mutex list_mutex_;
    std::vector < std::unique_ptr < SysdarftExternalDeviceBaseClass > > device_list;
    ControllerDataStream & ins(uint64_t port);
    void outs(uint64_t port, ControllerDataStream & buffer);
};

#endif //SYSDARFTIOHUB_H
