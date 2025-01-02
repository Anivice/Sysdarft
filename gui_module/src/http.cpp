// http.cpp
#include "gui_module_head.h"

// Start the session
void http_session::run()
{
    // Keep `this` alive
    auto self = shared_from_this();

    // Asynchronously read an HTTP request into `req_`
    // (which is a member variable, not a local)
    boost::beast::http::async_read(
        stream_, buffer_, req_,
        [self](boost::beast::error_code ec, std::size_t bytes) {
            if(ec)
            {
                if(ec == boost::asio::error::operation_aborted)
                    return; // canceled
                log("[HTTP] Read error: ", ec.message(), "\n");
                return;
            }

            // Check if it's a WebSocket upgrade
            if(boost::beast::websocket::is_upgrade(self->req_))
            {
                self->do_websocket_upgrade();
            }
            else
            {
                // Normal HTTP request
                self->do_http_request();
            }
        }
    );
}

// --------------- WEBSOCKET UPGRADE ---------------
// Release the stream and spawn a websocket_session
void http_session::do_websocket_upgrade()
{
    // Move the socket out of our stream_
    auto socket = stream_.release_socket();

    // Create a separate websocket_session if you like
    // (with your own design)
    auto ws_sess = std::make_shared<websocket_session>(std::move(socket));

    // If you want to pass the same request data:
    ws_sess->run(std::move(req_));  // e.g., pass the original request in

    // Optionally register with the backend so it can track or close them
    owner_->register_websocket(ws_sess);
}

// --------------- REGULAR HTTP RESPONSE ---------------
void http_session::do_http_request()
{
    bool shutdown = false;
    // Build a response in `res_` (another member variable)
    res_.version(req_.version());
    res_.keep_alive(false);

    if(req_.method() == boost::beast::http::verb::get && req_.target() == "/")
    {
        res_.result(boost::beast::http::status::ok);
        res_.set(boost::beast::http::field::server, "Sysdarft");
        res_.set(boost::beast::http::field::content_type, "text/html");
        res_.body() = owner_->make_html_page();
    }
    else if(req_.method() == boost::beast::http::verb::get && req_.target() == "/shutdown")
    {
        log("[HTTP] Shutdown request received!\n");
        res_.result(boost::beast::http::status::ok);
        res_.set(boost::beast::http::field::server, "Sysdarft");
        res_.set(boost::beast::http::field::content_type, "text/plain");
        res_.body() = "Shutting down...";

        shutdown = true;
    }
    else
    {
        res_.result(boost::beast::http::status::not_found);
        res_.set(boost::beast::http::field::server, "Sysdarft");
        res_.set(boost::beast::http::field::content_type, "text/plain");
        res_.body() = "Not found.";
    }

    // Prepare the payload for writing
    res_.prepare_payload();

    // Perform the async write
    auto self = shared_from_this();
    boost::beast::http::async_write(
        stream_, res_,
        [self](boost::beast::error_code ec, std::size_t) {
            if(!ec)
            {
                // Optionally shut down the socket's send side
                boost::system::error_code ignored;
                self->stream_.socket().shutdown(
                    boost::asio::ip::tcp::socket::shutdown_send, ignored);
            }
        }
    );

    // Trigger shutdown
    if (shutdown) {
        owner_->cleanup();
    }
}
