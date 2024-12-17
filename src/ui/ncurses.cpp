#include <ui_curses.h>
#include <thread>
#include <chrono>
#include <global_event.h>
#include <csignal>
#include <unistd.h>
#include <termios.h>
#include <atomic>
#include <ncurses.h>
#include <string>

std::atomic<bool> needs_refresh;

void ui_curses::run()
{
    set_thread_name("UI Runner");
    constexpr int targetFPS = 120;
    const auto frameDuration = std::chrono::milliseconds(1000 / targetFPS);

    while (running_thread_current_status.load())
    {
        auto start = std::chrono::steady_clock::now();

        if (needs_refresh.load())
        {
            needs_refresh.store(false);

            // Get new dimensions
            int new_rows, new_cols;
            getmaxyx(stdscr, new_rows, new_cols);

            // Adjust ncurses internal structures
            if (is_term_resized(new_rows, new_cols)) {
                resize_term(new_rows, new_cols);
            }

            // Clear and redraw
            clear();
            {
                std::lock_guard lock(memory_access_mutex);
                // Draw only up to new_rows, new_cols to avoid out-of-bounds
                for (unsigned int y = 0; y < HEIGHT && y < (unsigned int)new_rows; y++) {
                    for (unsigned int x = 0; x < WIDTH && x < (unsigned int)new_cols; x++) {
                        mvaddch(y, x, video_memory[x][y]);
                    }
                }

                auto [x, y] = cursor_pos.load();
                move(y, x);
            }

            refresh();
        }

        if (video_memory_changed.load())
        {
            std::lock_guard lock(memory_access_mutex);

            // Get current window size to avoid out-of-bounds
            int max_rows, max_cols;
            getmaxyx(stdscr, max_rows, max_cols);

            for (unsigned int y = 0; y < HEIGHT && y < (unsigned int)max_rows; y++) {
                for (unsigned int x = 0; x < WIDTH && x < (unsigned int)max_cols; x++) {
                    mvaddch(y, x, video_memory[x][y]);
                }
            }

            curs_set(TRUE);
            auto [x, y] = cursor_pos.load();
            move(y, x);
            refresh();
            video_memory_changed.store(false);
        }

        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (auto sleepDuration = frameDuration - elapsed;
            sleepDuration > std::chrono::milliseconds(0))
        {
            std::this_thread::sleep_for(sleepDuration);
        }
    }

    running_thread_current_exited.store(true);
}

void ui_curses::monitor_input()
{
    set_thread_name("UI Input Monitor");
    nodelay(stdscr, TRUE);

    while (monitor_input_status.load())
    {
        int input = getch();

        if (input == KEY_RESIZE) {
            needs_refresh.store(true);
        } else if (input == ERR) {
            continue;
        } else {
            GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_MONITOR_METHOD_NAME)(input);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    monitor_input_exited.store(true);
}

void ui_curses::start_curses()
{
    initscr();
    noecho();
    cbreak();
    curs_set(TRUE);
    keypad(stdscr, TRUE);
}

// Define the request handler type for clarity
using ReqHandler = std::function<void(int)>;

// RequestHandler class to manage rate limiting
class RequestHandler {
public:
    // Constructor takes the original handler and the masking duration
    RequestHandler(ReqHandler handler, std::chrono::milliseconds mask_duration)
        : handler_(std::move(handler)),
          mask_duration_(mask_duration),
          last_invocation_time_(std::chrono::steady_clock::time_point::min()) {}

    // Operator() to handle incoming requests
    void operator()(int request) {
        auto now = std::chrono::steady_clock::now();

        // Lock the mutex to ensure thread-safe access to last_invocation_time_
        std::lock_guard<std::mutex> lock(mutex_);

        // Check if enough time has passed since the last invocation
        if (now - last_invocation_time_ >= mask_duration_) {
            // Update the last invocation time
            last_invocation_time_ = now;

            // Unlock before calling the handler to prevent blocking other requests
            // This is achieved by creating a temporary copy of the handler
            // and releasing the lock before invoking it
            ReqHandler current_handler = handler_;
            lock.~lock_guard(); // Explicitly release the lock

            // Call the original handler outside the locked section
            current_handler(request);
        }
        // If the request arrives within the mask_duration_, it is ignored
    }

private:
    ReqHandler handler_;                                // Original handler
    std::chrono::milliseconds mask_duration_;           // Masking duration
    std::chrono::steady_clock::time_point last_invocation_time_; // Last invocation timestamp
    std::mutex mutex_;                                   // Mutex for thread safety
};

void sig_handle(int sig)
{
    if (sig == SIGWINCH) {
        needs_refresh.store(true);
    }
}

RequestHandler handler(sig_handle, std::chrono::milliseconds(500));

void sig_handle_proxy(int sig)
{
    handler(sig);
}

void ui_curses::initialize()
{
    if_i_cleaned_up = false;

    memory_access_mutex.lock();
    for (unsigned int y = 0; y < HEIGHT; y++)
    {
        for (unsigned int x = 0; x < WIDTH; x++) {
            video_memory[x][y] = ' ';
        }
    }
    video_memory_changed = true;
    memory_access_mutex.unlock();

    // Initialize ncurses
    start_curses();
    refresh();

    // Setup sigaction for SIGWINCH
    struct sigaction sa{};
    sa.sa_handler = sig_handle_proxy;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGWINCH, &sa, nullptr) == -1) {
        throw SysdarftBaseError("Installing signal handler failed!");
    }

    set_cursor(0, 0);
    set_cursor_visibility(false);
    curs_set(0);

    std::thread Worker1(&ui_curses::run, this);
    std::thread Worker2(&ui_curses::monitor_input, this);

    // Detach threads so they run independently
    Worker1.detach();
    Worker2.detach();
}

void ui_curses::cleanup()
{
    if (if_i_cleaned_up) {
        return;
    }

    std::lock_guard lock(memory_access_mutex);

    monitor_input_status.store(false);
    running_thread_current_status.store(false);
    endwin(); // End ncurses mode

    // Wait for threads to exit
    for (uint32_t try_ = 0; try_ < 100; try_++)
    {
        if (running_thread_current_exited.load() && monitor_input_exited.load()) {
            debug::log("Ncurses normal quit...\n");
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    debug::log("Ncurses waiting for quit timed out!\n");
}

void ui_curses::set_cursor(const int x, const int y)
{
    std::lock_guard lock(memory_access_mutex);
    // update position
    const cursor_position_t pos = { .x = x, .y = y };
    cursor_pos.store(pos);
}

cursor_position_t ui_curses::get_cursor()
{
    std::lock_guard lock(memory_access_mutex);
    return cursor_pos.load();
}

void ui_curses::display_char(int x, int y, int ch)
{
    std::lock_guard lock(memory_access_mutex);
    video_memory[x][y] = ch;
    video_memory_changed.store(true);
}

void ui_curses::set_cursor_visibility(bool visible)
{
    std::lock_guard lock(memory_access_mutex);
    curs_set(visible);
}
