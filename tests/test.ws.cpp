#include <chrono>
#include <crow.h>
#include <thread>

int main()
{
    crow::SimpleApp app;

    // Define a route that streams data
    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([&](crow::websocket::connection &conn) {
            CROW_LOG_INFO << "new websocket connection from " << conn.get_remote_ip();
        })

        .onclose([&](crow::websocket::connection &conn, const std::string &reason) {
            CROW_LOG_INFO << "websocket connection closed: " << reason;
        })

        .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool is_binary)
        {
            CROW_LOG_INFO << "Recv message:" << data;
            if (is_binary)
                conn.send_binary(data);
            else
                conn.send_text(data);
        });

    app.port(18080).run(); // Run the server on port 18080
}
