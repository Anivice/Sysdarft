#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_showContext()
{
    CROW_ROUTE(JSONBackend, "/ShowContext").methods(crow::HTTPMethod::GET)([this]()
    {
        json response;
        response["Version"] = SYSDARFT_VERSION;
        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());

        if (!breakpoint_triggered || args == nullptr) {
            response["Result"] = "Not Ready";
            return crow::response(400, response.dump());
        }

        response["Result"] = show_context(CPUInstance, actual_ip, opcode, *args.load());
        return crow::response{response.dump()};
    });
}
