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
