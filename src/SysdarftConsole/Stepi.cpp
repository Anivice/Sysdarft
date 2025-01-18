#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_stepi()
{
    CROW_ROUTE(JSONBackend, "/Stepi").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req)
    {
        // Create a JSON object using nlohmann/json
        json response;
        response["Version"] = SYSDARFT_VERSION;
        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());

        std::string bodyString = req.body;
        json clientJson;
        try {
            clientJson = json::parse(bodyString);
        } catch (const std::exception&) {
            response["Result"] = "Invalid Argument";
            return crow::response(400, response.dump());
        }

        if  (!clientJson.contains("Expression"))
        {
            response["Result"] = "Invalid Argument (Missing expression)";
            return crow::response(400, response.dump());
        }

        try {
            std::string expression = clientJson["Expression"];
            capitalization(expression);
            if (expression.front() == 'S') {
                stepi = true;
                response["Result"] = "Step Instruction";
            } else {
                stepi = false;
                response["Result"] = "Running";
            }
        } catch (const std::exception & e) {
            response["Result"] = "Actionable Expression Error: " + std::string(e.what());
            return crow::response(400, response.dump());
        }

        return crow::response{response.dump()};
    });
}
