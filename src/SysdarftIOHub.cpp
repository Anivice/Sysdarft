#include <SysdarftIOHub.h>
#include <SysdarftDebug.h>

SysdarftExternalDeviceBaseClass & SysdarftIOHub::query_device_based_on_port(const uint64_t port)
{
    for (const auto & deviceMap : device_list)
    {
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

void SysdarftIOHub::ins(const uint64_t port, ControllerDataStream & buffer)
{
    auto & device = query_device_based_on_port(port);
    if (!device.request_read(port)) {
        throw SysdarftDeviceIOError("Input error on port " + std::to_string(port));
    }

    buffer = device.device_buffer.at(port);
}

void SysdarftIOHub::outs(const uint64_t port, const ControllerDataStream & buffer)
{
    auto & device = query_device_based_on_port(port);
    device.device_buffer.at(port) = buffer;

    if (!device.request_write(port)) {
        throw SysdarftDeviceIOError("Output error on port " + std::to_string(port));
    }
}
