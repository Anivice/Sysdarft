#include <chrono>
#include <csignal>
#include <cstring>
#include <ctime>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>

// ---------- NCURSES -----------
#include <ncurses.h>

// ========== 1) The Rate-Limiting RequestHandler ==========
using ReqHandler = std::function<void(int)>;

class RequestHandler {
public:
    RequestHandler(ReqHandler handler, std::chrono::milliseconds mask_duration)
        : handler_(std::move(handler))
        , mask_duration_(mask_duration)
        , last_invocation_time_(std::chrono::steady_clock::time_point::min())
    {}

    void operator()(int request) {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);

        if (now - last_invocation_time_ >= mask_duration_) {
            last_invocation_time_ = now;

            // Unlock before calling the handler
            ReqHandler local_handler = handler_;
            lock.~lock_guard();

            local_handler(request);
        }
    }

private:
    ReqHandler handler_;
    std::chrono::milliseconds mask_duration_;
    std::chrono::steady_clock::time_point last_invocation_time_;
    std::mutex mutex_;
};

// ========== 2) Global Constants for "Video Memory" ==========
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

// ========== 3) Global Data Structures & Mutex ==========
std::mutex g_data_mutex;

// Virtual "video memory"
static char video_memory[V_WIDTH][V_HEIGHT];

// Offsets for rendering
static int offset_x = 0;
static int offset_y = 0;

// ========== 4) Forward Declarations ==========
void init_video_memory();
void render_screen();
void recalc_offsets(int rows, int cols);

// ========== 5) Our masked request handler for "RESIZE" ==========
// This function will be invoked only if enough time has passed (500 ms).
// We'll do the actual refresh & re-render from here.
void on_resize_request(int /*unused*/) {
    std::lock_guard<std::mutex> lock(g_data_mutex);
    // Refresh ncurses internal structures
    endwin();
    refresh();
    render_screen();
    refresh();
}

// ========== 6) Main Function ==========
int main()
{
    // 1) Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);

    // 2) Non-blocking input so we can loop and check timing
    nodelay(stdscr, true);

    // 3) Enable mouse events
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    // 4) Initialize the "video memory"
    {
        std::lock_guard<std::mutex> lock(g_data_mutex);
        init_video_memory();
    }

    // 5) Create a rate-limiting wrapper for resizing requests (500 ms mask)
    RequestHandler resize_handler(on_resize_request, std::chrono::milliseconds(500));

    // 6) Draw the initial screen
    {
        std::lock_guard<std::mutex> lock(g_data_mutex);
        render_screen();
    }
    refresh();

    bool done = false;
    while (!done)
    {
        // Read input if available
        int ch = getch();
        if (ch != ERR) {
            // We got an event
            if (ch == 'q') {
                done = true;
            }
            else if (ch == 'm') {
                // Example: modify the video memory
                std::lock_guard<std::mutex> lock(g_data_mutex);
                const char* text = "Modified!";
                int tx = 5, ty = 10;
                for (int i = 0; i < (int)std::strlen(text); i++) {
                    if (tx + i < V_WIDTH && ty < V_HEIGHT) {
                        video_memory[tx + i][ty] = text[i];
                    }
                }
                // Re-render right away
                render_screen();
                refresh();
            }
            else if (ch == KEY_MOUSE) {
                MEVENT me;
                if (getmouse(&me) == OK) {
                    // Check for wheel up/down
                    // (BUTTON4_PRESSED = wheel up, BUTTON5_PRESSED = wheel down)
                    std::lock_guard<std::mutex> lock(g_data_mutex);
                    if (me.bstate & BUTTON4_PRESSED) {
                        // Scroll up
                        if (offset_y > 0) {
                            offset_y--;
                            render_screen();
                            refresh();
                        }
                    }
                    else if (me.bstate & BUTTON5_PRESSED) {
                        // Scroll down
                        offset_y++;
                        render_screen();
                        refresh();
                    }
                }
            }
            else if (ch == KEY_RESIZE) {
                // Instead of redrawing immediately, we issue a "resize request"
                // that is rate-limited by our RequestHandler.
                resize_handler(1 /*dummy request code*/);
            }
        }

        // Just rest a little so we don't burn 100% CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    endwin();
    return 0;
}

// ========== 7) init_video_memory ==========
void init_video_memory()
{
    for (int y = 0; y < V_HEIGHT; y++) {
        for (int x = 0; x < V_WIDTH; x++) {
            video_memory[x][y] = '.';
        }
    }
    // Place a sample message
    const char* msg = "Hello from video_memory (C++23 + ncurses + RequestHandler)!";
    int msg_len = (int)std::strlen(msg);
    int mid_x   = (V_WIDTH  - msg_len) / 2;
    int mid_y   = (V_HEIGHT - 1) / 2;
    for (int i = 0; i < msg_len; i++) {
        video_memory[mid_x + i][mid_y] = msg[i];
    }
}

// ========== 8) render_screen ==========
// Clears the screen and draws the portion of video_memory that fits
void render_screen()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    recalc_offsets(rows, cols);

    clear(); // always clear first

    // Determine how many characters to print
    int max_x = (cols >= V_WIDTH)  ? V_WIDTH  : cols;
    int max_y = (rows >= V_HEIGHT) ? V_HEIGHT : rows;

    for (int y = 0; y < max_y; y++) {
        for (int x = 0; x < max_x; x++) {
            int sx = offset_x + x;
            int sy = offset_y + y;
            if (sx >= 0 && sx < cols && sy >= 0 && sy < rows) {
                mvaddch(sy, sx, video_memory[x][y]);
            }
        }
    }
}

// ========== 9) recalc_offsets ==========
// Decide where to place our virtual screen in the real terminal.
void recalc_offsets(int rows, int cols)
{
    // Horizontal centering if there's enough space; clamp otherwise
    if (cols >= V_WIDTH) {
        offset_x = (cols - V_WIDTH) / 2;
    } else {
        if (offset_x + V_WIDTH > cols) {
            offset_x = cols - V_WIDTH;
        }
        if (offset_x < 0) {
            offset_x = 0;
        }
    }

    // Vertical centering if there's enough space; clamp otherwise
    if (rows >= V_HEIGHT) {
        offset_y = (rows - V_HEIGHT) / 2;
    } else {
        if (offset_y + V_HEIGHT > rows) {
            offset_y = rows - V_HEIGHT;
        }
        if (offset_y < 0) {
            offset_y = 0;
        }
    }
}
