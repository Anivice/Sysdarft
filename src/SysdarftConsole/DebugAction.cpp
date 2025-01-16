#include <SysdarftMain.h>

std::string RemoteDebugServer::invoke_action(const std::vector < uint8_t > & action_code)
{
    {
        std::lock_guard<std::mutex> lock(action_access_mutex);
        debug_action_request = action_code;
    }

    std::string result;

    while (result.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::lock_guard<std::mutex> lock(action_result_access_mutex);
        result = debug_action_result;
    }

    {
        std::lock_guard<std::mutex> lock(action_result_access_mutex);
        debug_action_result.clear();
    }

    return result;
}

void RemoteDebugServer::at_breakpoint(__uint128_t,
    const uint64_t actual_ip,
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
                debug_action_request.clear();
                return;
            case SHOW_CONTEXT:
                debug_action_result = show_context(CPUInstance, actual_ip, opcode, args);
                debug_action_request.clear();
                continue;
            default: /* ignore any abnormal behavior */ continue;
            }
        }
    }

    debug_action_request.clear();
}
