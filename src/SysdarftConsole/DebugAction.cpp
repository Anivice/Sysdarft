#include <SysdarftMain.h>

void RemoteDebugServer::at_breakpoint(__uint128_t,
    uint64_t actual_ip,
    const uint8_t opcode,
    const SysdarftCPU::WidthAndOperandsType & args)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        {
            std::lock_guard lock(action_access_mutex);
            std::lock_guard lock2(action_result_access_mutex);

            if (debug_action_request.empty()) {
                continue;
            }

            switch (debug_action_request.at(0))
            {
                // NOTE: debug_action_result must be non-empty when return to avoid dead lock
            case CONTINUE:
                debug_action_result = "Continue";
                return;
            case SHOW_CONTEXT:
                debug_action_result = show_context(CPUInstance, actual_ip, opcode, args);
                break;
            default: /* ignore any abnormal behavior */ break;
            }

            debug_action_request.clear();
        }
    }
}
