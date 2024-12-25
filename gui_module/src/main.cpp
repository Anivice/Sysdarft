// backend_async.cpp
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

// -----------------------------------------------------------------------------
// 2) Utility function to get the shared library path (from your code)
std::string getSharedLibraryPath() {
    Dl_info dl_info;
    if (dladdr((void *)getSharedLibraryPath, &dl_info)) {
        if (dl_info.dli_fname) {
            return dl_info.dli_fname;
        }
    }
    return "";
}

class websocket_session;

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

public:
    backend()
        : ioc_()
          , acceptor_(ioc_)
          , videoBuffer_(V_HEIGHT, std::string(V_WIDTH, '.')) {
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

// -----------------------------------------------------------------------------
// 3) The async session for each WebSocket connection
//    - Owns a `websocket::stream`, a read buffer, etc.
//    - When we want to close it, we call `async_close` on the same I/O context.
class websocket_session : public std::enable_shared_from_this<websocket_session> {
public:
    explicit websocket_session(asio::ip::tcp::socket socket)
        : ws_(std::move(socket)) {
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    }

    // Start the asynchronous accept, then read loop.
    void run(http::request<http::string_body> req)
    {
        // We must accept the WebSocket handshake
        auto self = shared_from_this();
        ws_.async_accept(
            req,
            [self](beast::error_code ec) {
                if (ec) {
                    std::cerr << "[WebSocket] accept error: " << ec.message() << std::endl;
                    return;
                }
                self->do_read();
            }
            );
    }

    // Called to send data (e.g., from a render loop)
    void send_text(const std::string &text)
    {
        // Post to the same I/O context to ensure thread safety
        auto self = shared_from_this();
        asio::post(ws_.get_executor(),
                   [self, text]() {
                       // Queue the message in a buffer
                       self->outbox_.push_back(text);
                       if (!self->writing_) {
                           self->do_write();
                       }
                   }
            );
    }

    // Initiate an async close handshake
    void close()
    {
        auto self = shared_from_this();
        asio::post(ws_.get_executor(),
                   [self]() {
                       if (!self->closed_) {
                           self->closed_ = true;
                           beast::error_code ignored;
                           // We can start an async_close:
                           self->ws_.async_close(
                               websocket::close_code::normal,
                               [self](beast::error_code ec) {
                                   if (ec) {
                                       std::cerr << "[WebSocket] close error: "
                                           << ec.message() << std::endl;
                                   }
                               }
                               );
                       }
                   }
            );
    }

private:
    // Read loop
    void do_read()
    {
        auto self = shared_from_this();
        ws_.async_read(
            buffer_,
            [self](beast::error_code ec, std::size_t bytes_transferred) {
                beast::string_view data(
                    static_cast<const char *>(self->buffer_.data().data()),
                    self->buffer_.size());

                if (ec == websocket::error::closed) {
                    std::cout << "[WS] client closed websocket" << std::endl;
                    return; // normal closure
                } else if (ec) {
                    // Some error
                    std::cerr << "[WS] read error: " << ec.message() << std::endl;
                    return;
                }

                // We have "data"
                std::cout << "[WS] Received: " << data << std::endl;

                // consume buffer
                self->buffer_.consume(self->buffer_.size());

                // Keep reading
                self->do_read();
            }
            );
    }

    // Write loop (sends from outbox_)
    void do_write()
    {
        if (outbox_.empty()) {
            writing_ = false;
            return;
        }

        writing_ = true;
        auto message = outbox_.front();
        outbox_.pop_front();

        auto self = shared_from_this();
        ws_.text(true);
        ws_.async_write(
            asio::buffer(message),
            [self, message](beast::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "[WS] write error: " << ec.message() << std::endl;
                    return;
                }
                // Send next message if any
                self->do_write();
            }
            );
    }

    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

    bool closed_ = false; // track if we've initiated a close
    bool writing_ = false; // track if a write is in progress
    std::deque<std::string> outbox_; // queued messages
};

// -----------------------------------------------------------------------------
// 4) A class representing a single HTTP session (async).
//    - Either it serves an HTTP request or upgrades to WebSocket.
class http_session : public std::enable_shared_from_this<http_session> {
public:
    explicit http_session(asio::ip::tcp::socket socket, class backend *owner)
        : stream_(std::move(socket))
          , owner_(owner) {
    }

    void run()
    {
        auto self = shared_from_this();
        // We read the request from the socket
        http::async_read(stream_, buffer_, req_,
                         [self](beast::error_code ec, std::size_t) {
                             if (ec) {
                                 if (ec == asio::error::operation_aborted)
                                     return; // canceled
                                 std::cerr << "[HTTP] read error: " << ec.message() << std::endl;
                                 return;
                             }
                             // Check if WebSocket upgrade
                             if (websocket::is_upgrade(self->req_)) {
                                 // Create a WebSocket session
                                 self->do_websocket_upgrade(std::move(self->req_));
                             } else {
                                 // Normal HTTP
                                 self->handle_http_request(std::move(self->req_));
                             }
                         }
            );
    }

private:
    void do_websocket_upgrade(http::request<http::string_body> req)
    {
        // Release the stream's socket
        auto socket = stream_.release_socket();
        // Create a new websocket_session
        auto ws = std::make_shared<websocket_session>(std::move(socket));

        // Tell backend about it (so it can store or close them all on shutdown)
        owner_->register_websocket(ws);

        // Run it
        ws->run(std::move(req));
    }

    void handle_http_request(http::request<http::string_body> req) {
        // Build a response
        http::response<http::string_body> res;
        res.version(req.version());
        res.keep_alive(false);

        if (req.method() == http::verb::get && req.target() == "/") {
            res.result(http::status::ok);
            res.set(http::field::server, "Sysdarft");
            res.set(http::field::content_type, "text/html");
            res.body() = owner_->make_html_page();
        } else if (req.method() == http::verb::get && req.target() == "/shutdown") {
            std::cout << "Shutdown request received!" << std::endl;
            res.result(http::status::ok);
            res.set(http::field::server, "Sysdarft");
            res.set(http::field::content_type, "text/plain");
            res.body() = "Shutting down...";

            // Trigger shutdown
            owner_->request_shutdown();
        } else {
            res.result(http::status::not_found);
            res.set(http::field::server, "Sysdarft");
            res.set(http::field::content_type, "text/plain");
            res.body() = "Not found.";
        }

        // Write the response
        auto self = shared_from_this();
        http::async_write(stream_, res,
                          [self](beast::error_code ec, std::size_t) {
                              self->stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
                          }
            );
    }

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    class backend *owner_; // pointer back to the backend
};

// -----------------------------------------------------------------------------
// 5) The main "backend" class
// Called once: start acceptor + run ioc in a thread, start render thread
void backend::initialize()
{
    debug::log("main thread started!\n");

    // 1) Start accepting connections on port 8080 (async style)
    beast::error_code ec;
    tcp::endpoint endpoint(tcp::v4(), 8080);

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        std::cerr << "Acceptor open: " << ec.message() << std::endl;
        return;
    }
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    acceptor_.bind(endpoint, ec);
    if (ec) {
        std::cerr << "Acceptor bind: " << ec.message() << std::endl;
        return;
    }
    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        std::cerr << "Acceptor listen: " << ec.message() << std::endl;
        return;
    }

    // Start async accept
    start_accept();

    // 2) Launch a thread to run the io_context so async ops happen
    std::thread ioThread([this] {
        try {
            ioc_.run();
        } catch (std::exception &e) {
            std::cerr << "[IO Thread] exception: " << e.what() << std::endl;
        }
    });
    ioThread.detach();

    // 3) Launch the render loop in a separate thread
    renderThread_ = std::thread(&backend::render_loop, this);

    debug::log("Backend initialized!\n");
}

// Called on program exit or plugin unload
void backend::cleanup()
{
    if (shuttingDown_.exchange(true)) {
        // Already shutting down
        return;
    }
    debug::log("Sending shutdown request!\n");

    // Request shutdown
    request_shutdown();

    // Wait for render loop to exit
    if (renderThread_.joinable()) {
        renderThread_.join();
    }

    // The ioc_ won't stop until all async ops are done, but at least we
    // close the acceptor to avoid new connections
    beast::error_code ec;
    acceptor_.close(ec);

    // Force close all websockets
    close_all_websockets();

    // Post a stop event to ioc_
    ioc_.stop();

    debug::log("Backend cleanup complete!\n");
}

// Called from "/shutdown" or your own code
void backend::request_shutdown()
{
    shuttingDown_ = true;

    // 1) Stop accepting
    beast::error_code ec;
    acceptor_.close(ec);

    // 2) Stop render loop
    runLoop_ = false;

    // 3) Close websockets
    close_all_websockets();
}

// Return your fallback page or index.html contents
std::string backend::make_html_page()
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

// Register a new WebSocket session so we can close them on shutdown
void backend::register_websocket(std::shared_ptr<websocket_session> ws) {
    std::lock_guard<std::mutex> lock(wsMutex_);
    websockets_.push_back(ws);
}

// Start async accept
void backend::start_accept() {
    auto socket = std::make_shared<tcp::socket>(ioc_);
    acceptor_.async_accept(*socket,
                           [this, socket](beast::error_code ec) {
                               if (!ec && !shuttingDown_) {
                                   // Create http_session
                                   std::make_shared<http_session>(std::move(*socket), this)->run();
                               }
                               // Accept again if not shutting down
                               if (!shuttingDown_) {
                                   start_accept();
                               }
                           }
        );
}

// The render loop: updates video buffer, sends frames to all websockets
void backend::render_loop() {
    int frameCount = 0;
    while (runLoop_) {
        frameCount++;

        // Build a text frame
        {
            std::lock_guard<std::mutex> lk(videoMutex_);
            videoBuffer_[0] = "Frame: " + std::to_string(frameCount);
            videoBuffer_[0].resize(V_WIDTH, ' ');
        }

        std::string text;
        {
            std::lock_guard<std::mutex> lk(videoMutex_);
            for (auto &row : videoBuffer_) {
                text += row + "\n";
            }
        }

        // Send text to all websockets
        broadcast(text);

        std::this_thread::sleep_for(5ms);
    }
    std::cout << "[Render] Exiting loop\n";
}

// Send a message to all WebSocket sessions
void backend::broadcast(const std::string &text) {
    std::lock_guard<std::mutex> lock(wsMutex_);
    // Clean up any that have died
    auto it = websockets_.begin();
    while (it != websockets_.end()) {
        auto sp = it->lock();
        if (!sp) {
            it = websockets_.erase(it);
        } else {
            // async send
            sp->send_text(text);
            ++it;
        }
    }
}

// Close all websockets gracefully
void backend::close_all_websockets() {
    std::lock_guard<std::mutex> lock(wsMutex_);
    for (auto &weak : websockets_) {
        if (auto sp = weak.lock()) {
            sp->close(); // calls async_close
        }
    }
    websockets_.clear();
}

// dummy
void backend::set_cursor(int, int) {
}

cursor_position_t backend::get_cursor() { return {0, 0}; }

void backend::display_char(int, int, int) {
}

void backend::set_cursor_visibility(bool) {
}


// -----------------------------------------------------------------------------
// 6) The actual global object, plus C ABI for module_init / module_exit
static backend dummy;

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