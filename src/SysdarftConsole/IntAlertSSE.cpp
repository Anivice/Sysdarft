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

        .onclose([&](crow::websocket::connection &conn, const std::string &reason) {
            CROW_LOG_INFO << "Websocket connection closed: " << reason;
        })

        .onmessage([&](crow::websocket::connection &conn, const std::string &, bool is_binary)
        {
            const std::string data = breakpoint_triggered ? "Paused" : "Running";

            if (is_binary) {
                conn.send_binary(data);
            } else {
                conn.send_text(data);
            }
        });
}
