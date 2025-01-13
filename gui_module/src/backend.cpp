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
    if (is_instance_initialized_before) {
        log("[Backend] Creating multiple instances is NOT allowed! Exiting...\n");
        return;
    }

    is_instance_initialized_before = true;

    // 1) Start accepting connections on port 8080 (async style)
    beast::error_code ec;
    tcp::endpoint endpoint(tcp::v4(), 8080);

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        log("[Backend] Acceptor open: ", ec.message(), "\b");
        return;
    }
    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    acceptor_.bind(endpoint, ec);
    if (ec) {
        log("[Backend] Acceptor bind: ", ec.message(), "\n");
        return;
    }
    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        log("[Backend] Acceptor listen: ", ec.message(), "\n");
        return;
    }

    log("[Backend] Endpoint created! TCP listening on port 8080...\n");
    // Start async accept
    start_accept();

    // 2) Launch a thread to run the io_context so async ops happen
    std::thread([this] {
        set_thread_name("IO Thread");
        try {
            ioThreadExited = false;
            ioc_.run();
        } catch (std::exception &e) {
            log("[IO Thread] exception: ", e.what(), "\n");
        }

        log("[IO Thread] Thread exited!\n");
        ioThreadExited = true;
    }).detach();

    // 3) Launch the render loop in a separate thread
    renderThread_ = std::thread(&backend::render_loop, this);

    // 4) Start cursor worker
    start_cursor_worker();

    log("[Backend] Backend initialized! Instance created at http://127.0.0.1:8080\n");
}

// Called on program exit or plugin unload
void backend::cleanup()
{
    if (shuttingDown_.exchange(true)) {
        // Already shutting down
        return;
    }
    log("[Backend] Sending shutdown request!\n");
    // Request shutdown
    request_shutdown();

    log("[Backend] Request sent, waiting for threads to finish!\n");
    // Wait for render loop to exit
    if (renderThread_.joinable()) {
        renderThread_.join();
    }
    log("[Backend] Threads finished!\n");

    log("[Backend] Shutting down cursor worker!\n");
    // Stop cursor worker
    stop_cursor_worker();
    log("[Backend] Cursor worker shutdown complete!\n");

    log("[Backend] Shutting down acceptor!\n");
    // The ioc_ won't stop until all async ops are done, but at least we
    // close the acceptor to avoid new connections
    beast::error_code ec;
    acceptor_.close(ec);
    log("[Backend] Acceptor stopped!\n");

    log("[Backend] Close all websockets!\n");
    // Force close all websockets
    close_all_websockets();
    log("[Backend] All websockets terminated!\n");

    log("[Backend] Stopping event to IOC!\n");
    // Post a stop event to ioc_
    ioc_.stop();
    while (!ioThreadExited) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    log("[Backend] Stopped event to IOC!\n");

    log("[Backend] Backend cleanup complete!\n");
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
        log("[Backend] Failed to open webpage ", webpage, ", using embedded page!\n");
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
void backend::register_websocket(std::shared_ptr<websocket_session> ws)
{
    std::lock_guard<std::mutex> lock(wsMutex_);
    websockets_.push_back(ws);
}

// Start async accept
void backend::start_accept()
{
    auto socket = std::make_shared<tcp::socket>(ioc_);
    acceptor_.async_accept(*socket,
                           [this, socket](beast::error_code ec)
                           {
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
void backend::render_loop()
{
    set_thread_name("UI Render");

    int frameCount = 0;
    while (runLoop_) {
        frameCount++;

        // Build a text frame
        {
            std::lock_guard<std::mutex> lk(videoMutex_);
            auto now = std::chrono::system_clock::now();
            auto epoch = now.time_since_epoch();
            const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
            const auto seconds = milliseconds / 1000;
            const auto microseconds = milliseconds % 1000;
            videoBuffer_[0] = "Frame: " + std::to_string(frameCount)
                + " Timestamp: " + std::to_string(seconds) + "." + std::to_string(microseconds);
            videoBuffer_[0].resize(V_WIDTH, ' ');
        }

        // convert
        if (video_memory_changed)
        {
            std::lock_guard<std::mutex> lk(videoMutex_);
            std::lock_guard<std::mutex> lk2(BinaryBufferMutex);
            int y = 0;
            for (auto it = videoBuffer_.begin() + 1;
                it != videoBuffer_.end(); ++it, ++y)
            {
                for (int x = 0; x < V_WIDTH; ++x)
                {
                    auto ch = static_cast<char>(binary_video_buffer[x][y]);
                    if (ch < 0x20 || ch > 0x7e) { // character not displayable
                        it->at(x) = '.';
                    } else {
                        it->at(x) = ch;
                    }
                }
            }

            video_memory_changed = false;
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

    log("[Render] Exiting loop\n");
}

// Send a message to all WebSocket sessions
void backend::broadcast(const std::string &text)
{
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
void backend::close_all_websockets()
{
    std::lock_guard<std::mutex> lock(wsMutex_);
    for (auto &weak : websockets_) {
        if (const auto sp = weak.lock()) {
            sp->close(); // calls async_close
        }
    }
    websockets_.clear();
}

void backend::swap_cursor_with_char_at_pos()
{
    set_thread_name("UI Cursor");
    while (cursor_worker_running)
    {
        if (cursor_visibility)
        {
            std::lock_guard<std::mutex> lock(BinaryBufferMutex);
            const auto [x, y] = cursor_pos_.load();
            if (cursor_position_is_cursor_char_itself) {
                binary_video_buffer[x][y] = char_at_cursor_position;
                cursor_position_is_cursor_char_itself = false;
            } else {
                // back up the current character
                char_at_cursor_position = binary_video_buffer[x][y];
                // set the position to cursor
                binary_video_buffer[x][y] = cursor_char;
                cursor_position_is_cursor_char_itself = true;
            }

            video_memory_changed = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    cursor_worker_finished = true;
}

void backend::set_cursor(int x, int y)
{
    cursor_pos_.store({x, y});
}

cursor_position_t backend::get_cursor()
{
    return cursor_pos_;
}

void backend::display_char(int x, int y, int ch)
{
    std::lock_guard<std::mutex> lock(BinaryBufferMutex);
    if (const auto cur_pos = cursor_pos_.load();
        cur_pos == cursor_position_t { .x = x, .y = y  })
    {
        cursor_position_is_cursor_char_itself = false; // force update
    }

    binary_video_buffer[x][y] = ch;
    video_memory_changed = true;
}

void backend::set_cursor_visibility(bool visibility)
{
    if (cursor_position_is_cursor_char_itself && !visibility)
    {
        std::lock_guard<std::mutex> lock(BinaryBufferMutex);
        const auto [x, y] = cursor_pos_.load();
        binary_video_buffer[x][y] = char_at_cursor_position;
    }

    cursor_visibility = visibility;
}
