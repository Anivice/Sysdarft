#include <nlohmann/json.hpp>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <SysdarftMain.h>

using namespace std::literals;
using json = nlohmann::json;

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

        std::vector < uint8_t > expression_byte_code;
        try {
            expression_byte_code =
                compile_conditional_expression_to_byte_code(clientJson["Condition"]);
        } catch (const std::exception & e) {
            response["Result"] = "Conditional Expression Error: " + std::string(e.what());
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

        bool have_i_processed = false;
        for (auto it = breakpoint_list.begin(); it != breakpoint_list.end(); ++it)
        {
            if ((it->first.first + it->first.second) == (IP + CB)) {
                it->second = expression_byte_code;
                have_i_processed = true;
                break;
            }
        }

        // if not found, add
        if (!have_i_processed) {
            breakpoint_list.emplace(std::pair(CB, IP), expression_byte_code);
        }

        response["Result"] = "Success";
        response["Linear"] = std::to_string(CB + IP);
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
        response["Result"] = "Success";

        response["Context"] = invoke_action(SHOW_CONTEXT);

        return crow::response{response.dump()};
    });

    CROW_ROUTE(JSONBackend, "/Continue").methods(crow::HTTPMethod::POST)([this]()
    {
        json response;
        response["Version"] = SYSDARFT_VERSION;
        const auto timeNow = std::chrono::system_clock::now();
        response["UNIXTimestamp"] = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            timeNow.time_since_epoch()).count());
        response["Result"] = "Success";

        invoke_action(CONTINUE);

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

bool RemoteDebugServer::if_breakpoint(__uint128_t)
{
    const auto CB = CPUInstance.load<CodeBaseType>();
    const auto IP = CPUInstance.load<InstructionPointerType>();

    // halt system at startup, which is exactly where the start of BIOS is located
    if (!skip_bios_ip_check) {
        skip_bios_ip_check = true; // skip next IP+CB == BIOS scenario
        return true;
    }

    for (auto & [breakpoint, condition] : breakpoint_list)
    {
        if ((CB + IP) == (breakpoint.first + breakpoint.second)) {
            return is_condition_met(condition);
        }
    }

    return false;
}
