#include "shared.h"

websocket_session::websocket_session(asio::ip::tcp::socket socket)
    : ws_(std::move(socket))
{
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
}

// Start the asynchronous accept, then read loop.
void websocket_session::run(http::request<http::string_body> req)
{
    // We must accept the WebSocket handshake
    auto self = shared_from_this();
    ws_.async_accept(
        req,
        [self](beast::error_code ec) {
            if (ec) {
                debug::log("[WebSocket] accept error: ", ec.message(), "\n");
                return;
            }
            self->do_read();
        }
        );
}

// Called to send data (e.g., from a render loop)
void websocket_session::send_text(const std::string &text)
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
void websocket_session::close()
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
                           [self](beast::error_code ec)
                           {
                               if (ec) {
                                   debug::log("[WebSocket] close error: ", ec.message(), "\n");
                               }
                           });
                   }
               }
        );
}

// Read loop
void websocket_session::do_read()
{
    auto self = shared_from_this();
    ws_.async_read(
        buffer_,
        [self](beast::error_code ec, std::size_t bytes_transferred)
        {
            beast::string_view data(
                static_cast<const char *>(self->buffer_.data().data()),
                self->buffer_.size());

            if (ec == websocket::error::closed) {
                debug::log("[WebSocket] client closed websocket\n");
                return; // normal closure
            } else if (ec) {
                // Some error
                debug::log("[WebSocket] read error: ", ec.message(), "\n");
                return;
            }

            // We have "data"
            debug::log("[WebSocket] Received: ", data, "\n");

            // consume buffer
            self->buffer_.consume(self->buffer_.size());

            // Keep reading
            self->do_read();
        });
}

// Write loop (sends from outbox_)
void websocket_session::do_write()
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
                debug::log("[WebSocket] write error: ", ec.message(), "\n");
                return;
            }
            // Send next message if any
            self->do_write();
        });
}
