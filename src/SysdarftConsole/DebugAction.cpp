#include <SysdarftMain.h>

void RemoteDebugServer::at_breakpoint(__uint128_t,
    const uint64_t actual_ip,
    const uint8_t opcode,
    const SysdarftCPU::WidthAndOperandsType & args)
{
    this->actual_ip = actual_ip;
    this->opcode = opcode;
    this->args = &args;

    while (breakpoint_triggered)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    this->args = nullptr;
}
