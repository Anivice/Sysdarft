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
public:
    std::vector < uint8_t > device_buffer;

    template < typename DataType >
    void push(const DataType & data)
    {
        for (unsigned int i = 0; i < sizeof(data); i++) {
            device_buffer.emplace_back(((uint8_t*)&data)[i]);
        }
    }

    template < typename DataType >
    DataType pop()
    {
        DataType data { };
        for (unsigned int i = 0; i < sizeof(data); i++)
        {
            if (device_buffer.empty()) {
                ((uint8_t*)&data)[i] = 0;
            } else {
                ((uint8_t*)&data)[i] = device_buffer.front();
                device_buffer.erase(device_buffer.begin());
            }
        }

        return data;
    }
};

class SysdarftExternalDeviceBaseClass
{
public:
    virtual ~SysdarftExternalDeviceBaseClass() = default;
    std::map < uint64_t /* IO Port */, ControllerDataStream > device_buffer;
    virtual bool request_read(uint64_t /* IO Port */) { return false; }
    virtual bool request_write(uint64_t /* IO Port */) { return false; }
};

class SYSDARFT_EXPORT_SYMBOL SysdarftIOHub
{
private:
    SysdarftExternalDeviceBaseClass & query_device_based_on_port(uint64_t);

protected:
    std::vector < std::unique_ptr < SysdarftExternalDeviceBaseClass > > device_list;
    void ins(uint64_t port, ControllerDataStream & buffer);
    void outs(uint64_t port, const ControllerDataStream & buffer);
};

#endif //SYSDARFTIOHUB_H
