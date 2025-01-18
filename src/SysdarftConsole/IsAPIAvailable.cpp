#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_isAPIAvailable()
{
    // Install backend handler
    CROW_ROUTE(JSONBackend, "/IsAPIAvailable")([]()
    {
        // Create a JSON object using nlohmann/json
        json response;
        response["Version"] = SYSDARFT_VERSION;
        response["Result"] = SYSDARFT_INFORMATION;

        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());

        // Return the JSON as a Crow response with the correct MIME type
        return crow::response{response.dump()};
    });
}
