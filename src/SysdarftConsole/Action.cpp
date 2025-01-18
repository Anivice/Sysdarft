#include <SysdarftMain.h>
#include <nlohmann/json.hpp>
#include "debugger_operand.h"

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_action()
{
    CROW_ROUTE(JSONBackend, "/Action").methods(crow::HTTPMethod::POST)
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
                remove_spaces(expression);
                capitalization(expression);

                if (expression.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                // Create iterators to traverse all matches
                const auto matches_begin = std::sregex_iterator(expression.begin(), expression.end(), target_pattern);
                const auto matches_end = std::sregex_iterator();

                std::vector < std::vector<uint8_t> > operands;

                // Iterate over all matches and process them
                for (std::sregex_iterator i = matches_begin; i != matches_end; ++i)
                {
                    std::vector<uint8_t> code;
                    auto match = i->str();
                    replace_all(match, "<", "");
                    replace_all(match, ">", "");
                    encode_target(code, match);
                    operands.push_back(code);
                }

                if (expression.empty()) {
                    throw std::invalid_argument("Invalid Expression");
                }

                if (expression.front() == 'B') {
                    manual_stop = true;
                }

                if (expression.front() == 'S')
                {
                    if (!breakpoint_triggered) {
                        response["Result"] = "Still running";
                        return crow::response(400, response.dump());
                    }

                    if (operands.size() != 2) {
                        throw std::invalid_argument("Invalid Expression: Expected 2 operands, provided "
                            + std::to_string(operands.size()));
                    }

                    debugger_operand_type operand1(CPUInstance, operands[0]);
                    debugger_operand_type operand2(CPUInstance, operands[1]);
                    operand1.set_val(operand2.get_val());
                    response["Result"] = "Success";
                }

                if (expression.front() == 'G')
                {
                    if (!breakpoint_triggered) {
                        response["Result"] = "Still running";
                        return crow::response(400, response.dump());
                    }

                    if (operands.size() != 1) {
                        throw std::invalid_argument("Invalid Expression: Expected 2 operands, provided "
                            + std::to_string(operands.size()));
                    }

                    debugger_operand_type operand1(CPUInstance, operands[0]);
                    std::stringstream ss;
                    ss << "0x" << std::uppercase << std::hex << operand1.get_val();
                    response["Result"] = ss.str();
                }
            } catch (const std::exception & e) {
                response["Result"] = "Actionable Expression Error: " + std::string(e.what());
                return crow::response(400, response.dump());
            }

            return crow::response{response.dump()};
        });
}
