#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_showBreakpoint()
{
    CROW_ROUTE(JSONBackend, "/ShowBreakpoint").methods(crow::HTTPMethod::GET)([this]()
    {
        json response;
        response["Version"] = SYSDARFT_VERSION;
        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());
        std::stringstream ss;

        ss << "[BREAKPOINT]" << std::endl;
        std::lock_guard lock(g_br_list_access_mutex);
        for (const auto& breakpoint : breakpoint_list)
        {
            ss  << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
                << breakpoint.first.first + breakpoint.first.second << ": ";
            for (const auto & e : breakpoint.second) {
                ss << e;
            }

            ss << std::endl;
        }

        ss << "[WATCHLIST]" << std::endl;
        for (const auto& watch : watch_list)
        {
            for (const auto & e : watch.first) {
                ss << e;
            }

            ss << std::endl;
        }

        ss << "[END]" << std::endl;

        response["Result"] = ss.str();
        return crow::response{response.dump()};
    });
}
