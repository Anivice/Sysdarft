#include "shared.h"

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
