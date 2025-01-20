#include <SysdarftMain.h>
#include <nlohmann/json.hpp>

using namespace std::literals;
using json = nlohmann::json;

void RemoteDebugServer::crow_setup_intAlertSSE()
{
    // Define a route that streams data
    CROW_WEBSOCKET_ROUTE(JSONBackend, "/intAlertSSE")
        .onopen([&](crow::websocket::connection &conn) {
            CROW_LOG_INFO << "New websocket connection from " << conn.get_remote_ip();
        })

        .onclose([&](crow::websocket::connection &, const std::string &reason) {
            CROW_LOG_INFO << "Websocket connection closed: " << reason;
        })

        .onmessage([&](crow::websocket::connection &conn, const std::string & request, bool is_binary)
        {
            auto send_data = [&](const std::string &data)
            {
                if (is_binary) {
                    conn.send_binary(data);
                } else {
                    conn.send_text(data);
                }
            };

            if (request == "Status") {
                const std::string data = breakpoint_triggered ? "Paused" : "Running";
                send_data(data);
            } else if (request == "VideoMemory") {
                try {
                    static char buffer [VIDEO_MEMORY_SIZE + 3] = { 0 };
                    std::memset(buffer, 0, VIDEO_MEMORY_SIZE + 3);
                    buffer[0] = '\'';
                    CPUInstance.read_memory(VIDEO_MEMORY_START, buffer + 1, VIDEO_MEMORY_SIZE);
                    buffer[VIDEO_MEMORY_SIZE + 1] = '\'';
                    send_data(buffer);
                } catch (const std::exception &e) {
                    conn.send_text("Error: " + std::string(e.what()));
                }
            } else if (!request.empty() && request.front() == 'I') {
                std::string data = request;
                data.erase(data.begin());
                const int ch = static_cast<int>(std::strtol(data.c_str(), nullptr, 10));
                if (CPUInstance.try_add_input(ch)) {
                    send_data("Success");
                } else {
                    send_data("Failed");
                }
            } else {
                if (is_binary) {
                    conn.send_binary("Request not recognized");
                } else {
                    conn.send_text("Request not recognized");
                }
            }
        });
}
