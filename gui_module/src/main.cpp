// main.cpp

#include "fallback.h"
#include <nlohmann/json.hpp>
#include <boost/process.hpp>
#include <thread>
#include <string>
#include <dlfcn.h>
#include <debug.h>
#include <global.h>
#include <cstdio>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <mutex>
#include <atomic>
#include <ui_curses.h>
#include <chrono>
#include <fstream>

std::string getSharedLibraryPath() {
    Dl_info dl_info;
    if (dladdr((void *)getSharedLibraryPath, &dl_info)) {
        if (dl_info.dli_fname) {
            return dl_info.dli_fname;
        }
    }
    return "";
}

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;
namespace bp = boost::process;

class backend {
private:
    std::vector<std::string> g_videoBuffer = std::vector(V_HEIGHT, std::string(V_WIDTH, '.'));
    std::atomic<bool> video_memory_changed = false;
    std::mutex g_videoMutex;
    using json = nlohmann::json;
    std::vector<std::weak_ptr<websocket::stream<beast::tcp_stream>>> g_sessions;
    std::mutex g_sessionsMutex;
    std::atomic<bool> g_runLoop{true};
    std::atomic<bool> g_runLoop_shutdownFinished{true};
    std::atomic<bool> g_shutdown{false};
    std::atomic<bool> g_shutdownFinished{true};
    asio::io_context ioc_;
    tcp::acceptor acceptor_;
    std::atomic<int> g_activeThreads{0};
    std::atomic<bool> g_renderThreadExiting{false};

    std::string make_html_page()
    {
        std::string prefix = getSharedLibraryPath();
        while (prefix.back() != '/') {
            prefix.pop_back();
        }

        std::string webpage = prefix + "/resources/index.html";
        std::ifstream infile(webpage);
        if (!infile.is_open()) {
            std::cerr << "Failed to open webpage " << webpage << ", using embedded page!" << std::endl;
            return fallback_page;
        }

        std::string page, line;
        while (std::getline(infile, line)) {
            page += line + "\n";
        }
        infile.close();

        return page;
    }

    void handle_http_request(beast::tcp_stream &stream, http::request<http::string_body> req)
    {
        if (req.method() == http::verb::get && req.target() == "/") {
            http::response<http::string_body> res{
                http::status::ok, req.version()};
            res.set(http::field::server, "Sysdarft");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = make_html_page();
            res.prepare_payload();

            beast::error_code ec;
            http::write(stream, res, ec);
        }
        else if (req.method() == http::verb::get && req.target() == "/shutdown") {
            std::cout << "Shutdown request received!" << std::endl;

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "Sysdarft");
            res.set(http::field::content_type, "text/plain");
            res.keep_alive(req.keep_alive());
            res.body() = "Shutting down...";
            res.prepare_payload();

            beast::error_code ec;
            http::write(stream, res, ec);

            g_shutdown = true;
        }
        else
        {
            http::response<http::string_body> res{
                http::status::not_found, req.version()};
            res.set(http::field::server, "Sysdarft");
            res.keep_alive(req.keep_alive());
            res.body() = "Not found\r\n";
            res.prepare_payload();

            beast::error_code ec;
            http::write(stream, res, ec);
        }
    }

    void main_thread()
    {
        // We increment the global thread count
        g_activeThreads.fetch_add(1, std::memory_order_relaxed);

        try {
            // Start the detached render thread
            std::thread(&backend::render_loop, this).detach();

            // Prepare acceptor
            acceptor_.open(tcp::v4());
            acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
            acceptor_.bind({tcp::v4(), 8080});
            acceptor_.listen();

            // Accept loop
            while (!g_shutdown) {
                tcp::socket socket(ioc_);
                boost::system::error_code ec;
                acceptor_.accept(socket, ec);

                if (ec == asio::error::operation_aborted || g_shutdown) {
                    // If acceptor is closed or we are shutting down, break
                    break;
                }
                if (!ec) {
                    // For each client, spawn a DETACHED session thread
                    std::thread(&backend::do_session, this, std::move(socket)).detach();
                }
            }

            // We’re done accepting. Now let’s close websockets and finalize
            close_all_websockets();

            // Close acceptor
            boost::system::error_code ec;
            acceptor_.close(ec);

        } catch (std::exception const &e) {
            std::cerr << "[main_thread] Exception: " << e.what() << "\n";
        }

        // Decrement the global thread count
        g_activeThreads.fetch_sub(1, std::memory_order_relaxed);
    }

    void render_loop()
    {
        g_activeThreads.fetch_add(1, std::memory_order_relaxed);

        std::cout << "[Render] Rendering loop starts" << std::endl;
        using clock = std::chrono::steady_clock;
        auto nextFrame = clock::now();

        while (g_runLoop) {
            nextFrame += std::chrono::milliseconds(5);

            {
                std::lock_guard<std::mutex> lock(g_videoMutex);
                static int frameCount = 0;
                frameCount++;
                g_videoBuffer[0] = "Frame: " + std::to_string(frameCount);
                g_videoBuffer[0].resize(V_WIDTH, ' ');
                // If you want to see changes in other lines, manipulate them here
                // e.g. g_videoBuffer[1][0] = '*';
            }

            // Build single string
            std::string text;
            {
                std::lock_guard<std::mutex> lock(g_videoMutex);
                for (const auto &row : g_videoBuffer) {
                    text += row + "\n";
                }
            }

            // Send to all connected clients
            {
                std::lock_guard<std::mutex> lock(g_sessionsMutex);
                auto it = g_sessions.begin();
                while (it != g_sessions.end()) {
                    if (auto session = it->lock()) {
                        beast::error_code ec;
                        session->text(true);
                        session->write(boost::asio::buffer(text), ec);
                        if (ec == websocket::error::closed) {
                            it = g_sessions.erase(it);
                        } else {
                            ++it;
                        }
                    } else {
                        it = g_sessions.erase(it);
                    }
                }
            }

            std::this_thread::sleep_until(nextFrame);
        }

        g_renderThreadExiting = true;
        g_activeThreads.fetch_sub(1, std::memory_order_relaxed);
    }

    void do_session(tcp::socket&& socket)
    {
        g_activeThreads.fetch_add(1, std::memory_order_relaxed);

        try {
            beast::tcp_stream stream(std::move(socket));
            beast::flat_buffer buffer;

            http::request<http::string_body> req;
            http::read(stream, buffer, req);

            if (websocket::is_upgrade(req)) {
                // spawn websocket
                do_websocket_session(stream.release_socket(), std::move(req));
            } else {
                // normal http
                handle_http_request(stream, std::move(req));
            }
        } catch (std::exception const &e) {
            std::cerr << "[Session] Exception: " << e.what() << "\n";
        }

        g_activeThreads.fetch_sub(1, std::memory_order_relaxed);
    }

    void do_websocket_session(tcp::socket &&socket, http::request<http::string_body>&& req)
    {
        g_activeThreads.fetch_add(1, std::memory_order_relaxed);

        auto ws = std::make_shared<websocket::stream<beast::tcp_stream>>(std::move(socket));
        ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

        boost::system::error_code ec;
        ws->accept(req, ec);
        if (ec) {
            std::cerr << "[WS] Accept error: " << ec.message() << std::endl;
            g_activeThreads.fetch_sub(1, std::memory_order_relaxed);
            return;
        }

        // Add to g_sessions
        {
            std::lock_guard<std::mutex> lock(g_sessionsMutex);
            g_sessions.push_back(ws);
        }

        beast::flat_buffer buffer;
        for (;;) {
            ws->read(buffer, ec);
            if (ec == websocket::error::closed) {
                std::cout << "[WS] WebSocket closed" << std::endl;
                break;
            } else if (ec) {
                std::cerr << "[WS] Read error: " << ec.message() << std::endl;
                break;
            }
            // ... handle message ...
            buffer.consume(buffer.size());
        }

        // Remove from g_sessions
        {
            std::lock_guard<std::mutex> lock(g_sessionsMutex);
            auto it = g_sessions.begin();
            while (it != g_sessions.end()) {
                if (it->expired() || it->lock().get() == ws.get()) {
                    it = g_sessions.erase(it);
                } else {
                    ++it;
                }
            }
        }

        g_activeThreads.fetch_sub(1, std::memory_order_relaxed);
    }

    // Clean way to request shutdown
    void request_shutdown()
    {
        if (!g_shutdown.exchange(true)) {
            // If we weren’t already shutting down:
            // 1. close acceptor to break out of accept() calls
            boost::system::error_code ec;
            acceptor_.close(ec);

            // 2. close websockets so they break out of read()
            close_all_websockets();

            // 3. stop render loop
            g_runLoop = false;
        }
    }

    void close_all_websockets()
    {
        std::lock_guard<std::mutex> lock(g_sessionsMutex);
        for (auto &weak : g_sessions) {
            if (auto s = weak.lock()) {
                boost::system::error_code ec;
                // Attempt graceful close
                s->close(websocket::close_code::normal, ec);
            }
        }
        g_sessions.clear();
    }

public:
    void cleanup()
    {
        // If we haven’t already shut down, do it now
        if (!g_shutdown) {
            debug::log("Sending shutdown request!\n");
            request_shutdown();
        }

        // Now wait until *all* threads have truly finished
        // (including session threads and render thread).
        // Because they are detached, we can't join them.
        // Instead, we poll the global counter.
        // If your plugin architecture is short-lived or you can't do a loop here,
        // you might skip or reduce the poll time.
        while (g_activeThreads.load() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        debug::log("All detached threads exited!\n");
        debug::log("Backend cleanup complete!\n");
    }

    void initialize()
    {
        std::thread(&backend::main_thread, this).detach();
        debug::log("main thread started!\n");
    }

    void set_cursor(int, int) {}
    cursor_position_t get_cursor() { return {0, 0}; }
    void display_char(int, int, int) {}
    void set_cursor_visibility(bool) {}
    backend() : ioc_() , acceptor_(ioc_) { }
} dummy;


extern "C" {
    int EXPORT module_init(void);
    void EXPORT module_exit(void);
    std::vector<std::string> EXPORT module_dependencies(void);
}

std::vector<std::string> EXPORT module_dependencies(void) {
    return {};
}

int EXPORT module_init(void) {
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_CLEANUP_METHOD_NAME, &backend::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_INITIALIZE_METHOD_NAME, &backend::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_SET_CURSOR_METHOD_NAME, &backend::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_GET_CURSOR_METHOD_NAME, &backend::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_DISPLAY_CHAR_METHOD_NAME, &backend::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &backend::set_cursor_visibility);
    debug::log("UI instance overridden!\n");
    return 0;
}

void EXPORT module_exit(void) {
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_CLEANUP_METHOD_NAME, &ui_curses::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_INITIALIZE_METHOD_NAME, &ui_curses::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_METHOD_NAME, &ui_curses::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_GET_CURSOR_METHOD_NAME, &ui_curses::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_DISPLAY_CHAR_METHOD_NAME, &ui_curses::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &ui_curses::set_cursor_visibility);
    debug::log("UI instance restored!\n");
    debug::log("Backend exited!\n");
}
