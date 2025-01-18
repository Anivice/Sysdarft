#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_continue()
{
    CROW_ROUTE(JSONBackend, "/Continue").methods(crow::HTTPMethod::POST)([this]()
        {
            json response;
            response["Version"] = SYSDARFT_VERSION;
            const auto timeNow = std::chrono::system_clock::now();
            response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                timeNow.time_since_epoch()).count());

            if (!breakpoint_triggered) {
                response["Result"] = "Already running";
                return crow::response(400, response.dump());
            }

            breakpoint_triggered = false;
            response["Result"] = "Success";
            return crow::response{response.dump()};
        });
}
