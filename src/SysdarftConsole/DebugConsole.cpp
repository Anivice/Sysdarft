#include <SysdarftMain.h>

bool RemoteDebugServer::if_breakpoint(__uint128_t)
{
    try {
        breakpoint_triggered = false;

        if (stepi) {
            breakpoint_triggered = true;
            return true;
        }

        if (manual_stop)
        {
            breakpoint_triggered = true;
            manual_stop = false;
            return true;
        }

        const auto CB = CPUInstance.load<CodeBaseType>();
        const auto IP = CPUInstance.load<InstructionPointerType>();

        // halt system at startup, which is exactly where the start of BIOS is located
        if (!skip_bios_ip_check && (IP + CB == BIOS_START)) {
            skip_bios_ip_check = true; // skip next IP+CB == BIOS scenario
            breakpoint_triggered = true;
            return true;
        }

        std::lock_guard lock(g_br_list_access_mutex);
        for (const auto & [breakpoint, condition] :
            breakpoint_list)
        {
            if ((CB + IP) == (breakpoint.first + breakpoint.second))
            {
                if (is_condition_met(condition)) {
                    breakpoint_triggered = true;
                    return true;
                }
            }
        }

        bool is_hit = false;
        for (auto & watch : watch_list)
        {
            if (const uint64_t data_this_time = absolute_target_access(watch.first);
                data_this_time != watch.second)
            {
                watch.second = data_this_time;
                is_hit = true;
            }
        }

        breakpoint_triggered = is_hit;
        return is_hit;
    } catch (std::exception &) {
        return true;
    }
}
