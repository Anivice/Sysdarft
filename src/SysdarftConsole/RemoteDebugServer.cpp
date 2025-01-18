#include "debugger_operand.h"

#include <SysdarftMain.h>
#include <chrono>
#include <fstream>
#include <list>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace std::literals;
using json = nlohmann::json;

inline void remove_spaces(std::string &input)
{
    input.erase(std::ranges::remove_if(input, ::isspace).begin(), input.end());
}

RemoteDebugServer::RemoteDebugServer(
    const std::string & ip,
    uint16_t port,
    SysdarftCPU & _CPUInstance,
    const std::string & crow_log_file)
    : CPUInstance(_CPUInstance), SysdarftLogHandlerInstance(crow_log, !crow_log_file.empty())
{
    // Setup Crow log handler
    try {
        if (!crow_log_file.empty()) {
            crow_log.open(crow_log_file);
        }

        crow::logger::setHandler(&SysdarftLogHandlerInstance);
        crow::logger::setLogLevel(crow::LogLevel::Debug);
    } catch (const std::exception & e) {
        std::cerr << "Error occurred while setting up log stream: " << e.what() << std::endl;
        throw;
    }

    // Install handler
    CPUInstance.bindIsBreakHere(this, &RemoteDebugServer::if_breakpoint);
    CPUInstance.bindBreakpointHandler(this, &RemoteDebugServer::at_breakpoint);

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
        breakpoint_list[std::pair(CB, IP)] = condition;

        response["Result"] = "Success";
        response["Linear"] = address_literal.str();
        // Return the JSON as a Crow response with the correct MIME type
        return crow::response{response.dump()};
    });

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

    CROW_ROUTE(JSONBackend, "/Watcher").methods(crow::HTTPMethod::POST)
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

        std::vector<uint8_t> target_byte_code;
        if (!clientJson.contains("Expression")) {
            response["Result"] = "Invalid Argument: Missing expression!";
            return crow::response(400, response.dump());
        }

        try {
            std::string expression = clientJson["Expression"];
            remove_spaces(expression);
            capitalization(expression);
            replace_all(expression, "<", "");
            replace_all(expression, ">", "");
            encode_target(target_byte_code, expression);
        } catch (const std::exception& e) {
            response["Result"] = "Invalid Argument: " + std::string(e.what());
            return crow::response(400, response.dump());
        }

        std::lock_guard lock(g_br_list_access_mutex);
        watch_list.emplace_back(target_byte_code, 0);
        response["Result"] = "Success";

        return crow::response{response.dump()};
    });

    server_thread = std::thread ([this](
        // DO NOT capture the current context, since it will cause `stack-use-after-return`
        const std::string & IP_,
        const uint16_t port_)
    {
        debug::set_thread_name("Crow Backend");
        JSONBackend.bindaddr(IP_).port(port_).multithreaded().run();
    }, ip, port);
}

RemoteDebugServer::~RemoteDebugServer()
{
    JSONBackend.stop();

    // Join the server thread to clean up
    if (server_thread.joinable()) {
        server_thread.join();
    }
}
