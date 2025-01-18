#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_setBreakpoint()
{
    CROW_ROUTE(JSONBackend, "/SetBreakpoint").methods(crow::HTTPMethod::POST)
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

        if  (  !clientJson.contains("CB")
            || !clientJson.contains("IP")
            || !clientJson.contains("Condition"))
        {
            response["Result"] = "Invalid Argument (Missing CB, IP, and/or Condition)";
            return crow::response(400, response.dump());
        }

        std::string CB_literal = clientJson["CB"];
        std::string IP_literal = clientJson["IP"];

        process_base16(CB_literal);
        process_base16(IP_literal);

        CB_literal = execute_bc(CB_literal);
        IP_literal = execute_bc(IP_literal);

        const auto CB = std::strtoull(CB_literal.c_str(), nullptr, 10);
        const auto IP = std::strtoull(IP_literal.c_str(), nullptr, 10);

        std::stringstream address_literal;
        address_literal << "0x" << std::uppercase << std::hex << CB + IP;

        std::string condition = clientJson["Condition"];
        std::string delete_check = condition;
        capitalization(delete_check);
        if (delete_check == "DELETE")
        {
            std::lock_guard lock(g_br_list_access_mutex);
            for (auto it = breakpoint_list.begin(); it != breakpoint_list.end(); ++it)
            {
                if ((it->first.first + it->first.second) == (IP + CB))
                {
                    breakpoint_list.erase(it);
                    response["Result"] = "Breakpoint deleted at: " + address_literal.str();
                    return crow::response(400, response.dump());
                }
            }

            response["Result"] = "No breakpoint found at: " + address_literal.str();
            return crow::response(400, response.dump());
        }

        try {
            if (!condition.empty()) {
                Parser parser(condition);
                parser.parseExpression(); // try parse once to do a sanity check
            }
        } catch (const std::exception & e) {
            response["Result"] = "Conditional Expression Error: " + std::string(e.what());
            return crow::response(400, response.dump());
        }

        std::lock_guard lock(g_br_list_access_mutex);
        try {
            breakpoint_list[std::pair(CB, IP)] = condition;
        } catch (const std::exception & e) {
            response["Result"] = "Conditional Expression Error: " + std::string(e.what());
            return crow::response(400, response.dump());
        }

        response["Result"] = "Success";
        response["Linear"] = address_literal.str();
        // Return the JSON as a Crow response with the correct MIME type
        return crow::response{response.dump()};
    });
}
