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
#include <unordered_map>
#include <cctype> // For std::tolower

using json = nlohmann::json;
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
    std::mutex BinaryBufferMutex;
    std::array < std::array < int, V_HEIGHT >, V_WIDTH > binary_video_buffer;

    std::atomic<bool> is_instance_initialized_before = {false};
    std::atomic<bool> video_memory_changed = {true};

    std::atomic<bool> ioThreadExited = {true};
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
        constexpr int mid_y   = (V_HEIGHT - 1) / 2;

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

    std::atomic<cursor_position_t> cursor_pos_ = { { 0, 0 } };
    std::atomic<int> char_at_cursor_position = '.';
    std::atomic<bool> cursor_position_is_cursor_char_itself = false;
    std::atomic<char> cursor_char = '_';
    std::atomic<bool> cursor_visibility = true;
    std::atomic<bool> cursor_worker_running = false;
    std::atomic<bool> cursor_worker_finished = true;
    void swap_cursor_with_char_at_pos();

    void start_cursor_worker()
    {
        cursor_worker_running = true;
        cursor_worker_finished = false;
        std::thread(&backend::swap_cursor_with_char_at_pos, this).detach();
        debug::log("[Cursor] Started cursor worker thread\n");
    }

    void stop_cursor_worker()
    {
        cursor_worker_running = false;
        while (!cursor_worker_finished) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        debug::log("[Cursor] Stopped cursor worker thread\n");
    }

public:
    // dummy
    void set_cursor(int, int);
    cursor_position_t get_cursor();
    void display_char(int, int, int);
    void set_cursor_visibility(bool);
};

/************************************************************
 *  ENUM DEFINITIONS
 ************************************************************/

// Modifier flags
enum ModifierFlag {
    MOD_NONE  = 0,
    MOD_CTRL  = 1 << 0, // 0x01
    MOD_SHIFT = 1 << 1, // 0x02
    MOD_ALT   = 1 << 2  // 0x04
    // Add more modifiers if needed
};

// Internal key codes
enum InternalKeyCode {
    // Printable characters mapped to ASCII codes
    KEY_SPACE       = 32,
    KEY_EXCLAMATION = 33,
    KEY_DOUBLE_QUOTE = 34,
    KEY_HASH        = 35,
    KEY_DOLLAR      = 36,
    KEY_PERCENT     = 37,
    KEY_AMPERSAND   = 38,
    KEY_APOSTROPHE  = 39,
    KEY_LEFT_PAREN  = 40,
    KEY_RIGHT_PAREN = 41,
    KEY_ASTERISK    = 42,
    KEY_PLUS        = 43,
    KEY_COMMA       = 44,
    KEY_MINUS       = 45,
    KEY_PERIOD      = 46,
    KEY_SLASH       = 47,
    KEY_0           = 48,
    KEY_1           = 49,
    KEY_2           = 50,
    KEY_3           = 51,
    KEY_4           = 52,
    KEY_5           = 53,
    KEY_6           = 54,
    KEY_7           = 55,
    KEY_8           = 56,
    KEY_9           = 57,
    KEY_COLON       = 58,
    KEY_SEMICOLON   = 59,
    KEY_LESS_THAN   = 60,
    KEY_EQUAL       = 61,
    KEY_GREATER_THAN = 62,
    KEY_QUESTION    = 63,
    KEY_AT          = 64,
    // Uppercase letters
    KEY_A           = 65,
    KEY_B           = 66,
    KEY_C           = 67,
    KEY_D           = 68,
    KEY_E           = 69,
    KEY_F           = 70,
    KEY_G           = 71,
    KEY_H           = 72,
    KEY_I           = 73,
    KEY_J           = 74,
    KEY_K           = 75,
    KEY_L           = 76,
    KEY_M           = 77,
    KEY_N           = 78,
    KEY_O           = 79,
    KEY_P           = 80,
    KEY_Q           = 81,
    KEY_R           = 82,
    KEY_S           = 83,
    KEY_T           = 84,
    KEY_U           = 85,
    KEY_V           = 86,
    KEY_W           = 87,
    KEY_X           = 88,
    KEY_Y           = 89,
    KEY_Z           = 90,
    KEY_LEFT_BRACKET = 91,
    KEY_BACKSLASH   = 92,
    KEY_RIGHT_BRACKET = 93,
    KEY_CARET       = 94,
    KEY_UNDERSCORE  = 95,
    KEY_BACKQUOTE   = 96,
    KEY_a           = 97,
    KEY_b           = 98,
    KEY_c           = 99,
    KEY_d           = 100,
    KEY_e           = 101,
    KEY_f           = 102,
    KEY_g           = 103,
    KEY_h           = 104,
    KEY_i           = 105,
    KEY_j           = 106,
    KEY_k           = 107,
    KEY_l           = 108,
    KEY_m           = 109,
    KEY_n           = 110,
    KEY_o           = 111,
    KEY_p           = 112,
    KEY_q           = 113,
    KEY_r           = 114,
    KEY_s           = 115,
    KEY_t           = 116,
    KEY_u           = 117,
    KEY_v           = 118,
    KEY_w           = 119,
    KEY_x           = 120,
    KEY_y           = 121,
    KEY_z           = 122,
    KEY_LEFT_BRACE   = 123,
    KEY_PIPE          = 124,
    KEY_RIGHT_BRACE  = 125,
    KEY_TILDE        = 126,

    // Special keys beyond ASCII range
    KEY_ARROW_UP     = 1000,
    KEY_ARROW_DOWN   = 1001,
    KEY_ARROW_LEFT   = 1002,
    KEY_ARROW_RIGHT  = 1003,
    KEY_ENTER        = 1004,
    KEY_BACKSPACE    = 1005,
    KEY_TAB          = 1006,
    KEY_ESCAPE       = 1007,
    KEY_DELETE       = 1008,
    // Add more special keys as needed

    KEY_ALT          = 1009,

    KEY_UNKNOWN      = -1
};

/************************************************************
 *  MAPPING TABLES
 ************************************************************/

// Mapping from 'key' string to InternalKeyCode
const std::unordered_map<std::string, InternalKeyCode> keyStringToCodeMap = {
    {" ", KEY_SPACE},
    {"!", KEY_EXCLAMATION},
    {"\"", KEY_DOUBLE_QUOTE},
    {"#", KEY_HASH},
    {"$", KEY_DOLLAR},
    {"%", KEY_PERCENT},
    {"&", KEY_AMPERSAND},
    {"'", KEY_APOSTROPHE},
    {"(", KEY_LEFT_PAREN},
    {")", KEY_RIGHT_PAREN},
    {"*", KEY_ASTERISK},
    {"+", KEY_PLUS},
    {",", KEY_COMMA},
    {"-", KEY_MINUS},
    {".", KEY_PERIOD},
    {"/", KEY_SLASH},
    {"0", KEY_0},
    {"1", KEY_1},
    {"2", KEY_2},
    {"3", KEY_3},
    {"4", KEY_4},
    {"5", KEY_5},
    {"6", KEY_6},
    {"7", KEY_7},
    {"8", KEY_8},
    {"9", KEY_9},
    {":", KEY_COLON},
    {";", KEY_SEMICOLON},
    {"<", KEY_LESS_THAN},
    {"=", KEY_EQUAL},
    {">", KEY_GREATER_THAN},
    {"?", KEY_QUESTION},
    {"@", KEY_AT},
    {"[", KEY_LEFT_BRACKET},
    {"]", KEY_RIGHT_BRACKET},
    {"^", KEY_CARET},
    {"_", KEY_UNDERSCORE},
    {"`", KEY_BACKQUOTE},
    {"{", KEY_LEFT_BRACE},
    {"|", KEY_PIPE},
    {"}", KEY_RIGHT_BRACE},
    {"~", KEY_TILDE},
    // Letters and digits are handled separately
    // Special keys
    {"ArrowUp", KEY_ARROW_UP},
    {"ArrowDown", KEY_ARROW_DOWN},
    {"ArrowLeft", KEY_ARROW_LEFT},
    {"ArrowRight", KEY_ARROW_RIGHT},
    {"Enter", KEY_ENTER},
    {"Backspace", KEY_BACKSPACE},
    {"Tab", KEY_TAB},
    {"Escape", KEY_ESCAPE},
    {"Delete", KEY_DELETE},
    {"Alt", KEY_ALT}
    // Add more special keys as needed
};

/************************************************************
 *  STRUCT DEFINITIONS
 ************************************************************/

// Structure to represent a key event
struct KeyEvent {
    int keyCode;     // ASCII code or special key code
    int modifiers;   // Bitmask flags for Ctrl, Shift, Alt

    explicit KeyEvent(const int code = KEY_UNKNOWN, const int mods = MOD_NONE)
        : keyCode(code), modifiers(mods) { }
};

/************************************************************
 *  FUNCTION DEFINITIONS
 ************************************************************/

/**
 * @brief Converts a JSON keydown event to an internal KeyEvent structure.
 *
 * @param jsonString The JSON string representing the keydown event.
 * @return KeyEvent The internal representation of the key event.
 */
KeyEvent convertJsonToKeyEvent(const std::string& jsonString);

#endif //SHARED_H
