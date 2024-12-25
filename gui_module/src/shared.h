#ifndef SHARED_H
#define SHARED_H

#include "fallback.h"
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <string>
#include <dlfcn.h>
#include <debug.h>
#include <global.h>
#include <cstdio>
#include <vector>
#include <mutex>
#include <atomic>
#include <ui_curses.h>
#include <fstream>
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;

using tcp = asio::ip::tcp;
using namespace std::chrono_literals;

class websocket_session : public std::enable_shared_from_this<websocket_session> {
public:
    explicit websocket_session(asio::ip::tcp::socket socket);

    // Start the asynchronous accept, then read loop.
    void run(http::request<http::string_body> req);

    // Called to send data (e.g., from a render loop)
    void send_text(const std::string &text);

    // Initiate an async close handshake
    void close();

private:
    // Read loop
    void do_read();

    // Write loop (sends from outbox_)
    void do_write();

    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

    bool closed_ = false; // track if we've initiated a close
    bool writing_ = false; // track if a write is in progress
    std::deque<std::string> outbox_; // queued messages
};

class http_session : public std::enable_shared_from_this<http_session> {
public:
    explicit http_session(asio::ip::tcp::socket socket, class backend *owner)
        : stream_(std::move(socket))
          , owner_(owner) { }

    void run();

private:
    void do_websocket_upgrade();
    void do_http_request();

    // Owns the I/O
    boost::beast::tcp_stream stream_;

    // Buffer for reading HTTP requests
    boost::beast::flat_buffer buffer_;

    // The HTTP request and response are stored here
    boost::beast::http::request<boost::beast::http::string_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;

    // Some pointer to your "backend" or application state
    backend* owner_;
};

class backend {
private:
    asio::io_context ioc_;
    tcp::acceptor acceptor_;

    // Our "render loop" thread
    std::thread renderThread_;
    std::atomic<bool> runLoop_{true};

    // Keep track of all websockets (shared_ptr) so we can close them
    std::mutex wsMutex_;
    std::vector<std::weak_ptr<websocket_session>> websockets_;

    std::atomic<bool> shuttingDown_{false};

    // For text "Frame: X" in the UI
    std::mutex videoMutex_;
    std::vector<std::string> videoBuffer_;
    /* raw buffer */
    /* needs to be converted and discard non-ASCII characters */
    std::array < std::array < int, V_HEIGHT >, V_WIDTH > binary_video_buffer;

    std::atomic<bool> is_instance_initialized_before = {false};

public:
    backend()
        : ioc_()
          , acceptor_(ioc_)
          , videoBuffer_(V_HEIGHT + 1, std::string(V_WIDTH, '.'))
    {
        for (int y = 0; y < V_HEIGHT; y++) {
            for (int x = 0; x < V_WIDTH; x++) {
                binary_video_buffer[x][y] = '.';
            }
        }

        // Place a message in the middle
        const char* msg = "(Video Memory Not Initialized)";
        const int msg_len     = static_cast<int>(std::strlen(msg));
        const int mid_x       = (V_WIDTH  - msg_len) / 2;
        constexpr int mid_y       = (V_HEIGHT - 1) / 2;

        for (int i = 0; i < msg_len; i++) {
            binary_video_buffer[mid_x + i][mid_y] = msg[i];
        }
    }

    // Called once: start acceptor + run ioc in a thread, start render thread
    void initialize();

    // Called on program exit or plugin unload
    void cleanup();

    // Called from "/shutdown" or your own code
    void request_shutdown();

    // Return your fallback page or index.html contents
    std::string make_html_page();

    // Register a new WebSocket session so we can close them on shutdown
    void register_websocket(std::shared_ptr<websocket_session> ws);

private:
    // Start async accept
    void start_accept();

    // The render loop: updates video buffer, sends frames to all websockets
    void render_loop();

    // Send a message to all WebSocket sessions
    void broadcast(const std::string &text);

    // Close all websockets gracefully
    void close_all_websockets();

public:
    // dummy
    void set_cursor(int, int);
    cursor_position_t get_cursor();
    void display_char(int, int, int);
    void set_cursor_visibility(bool);
};

#endif //SHARED_H
