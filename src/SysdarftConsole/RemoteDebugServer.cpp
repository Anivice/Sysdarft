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

    crow_setup_action();
    crow_setup_continue();
    crow_setup_showContext();
    crow_setup_intAlertSSE();
    crow_setup_isAPIAvailable();
    crow_setup_setBreakpoint();
    crow_setup_showBreakpoint();
    crow_setup_stepi();
    crow_setup_watcher();

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
