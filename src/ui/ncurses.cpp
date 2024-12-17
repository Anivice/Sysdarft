#include <ui_curses.h>
#include <thread>
#include <chrono>
#include <global_event.h>
#include <csignal>
#include <unistd.h>
#include <termios.h>
#include <atomic>
#include <ncurses.h>

std::atomic<bool> needs_refresh;

void ui_curses::run()
{
    // Target 30 FPS
    constexpr int targetFPS = 30;
    const auto frameDuration = std::chrono::milliseconds(1000 / targetFPS);
    uint32_t current_frame_within_seconds = 0;
    unsigned int new_cols = WIDTH, new_rows = HEIGHT;

    while (running_thread_current_status.load())
    {
        auto start = std::chrono::steady_clock::now();
        current_frame_within_seconds++;

        if (cursor_visibility && current_frame_within_seconds == 15)
        {
            std::lock_guard lock(memory_access_mutex);
            video_memory[cursor_pos.x][cursor_pos.y] = cursor_char.load();
            video_memory_changed = true;
            char_at_cursor_pos_is_not_cursor = false;
        }
        else if (current_frame_within_seconds == 30)
        {
            current_frame_within_seconds = 0;
            std::lock_guard lock(memory_access_mutex);
            video_memory[cursor_pos.x][cursor_pos.y] = char_at_cursor_position.load();
            video_memory_changed = true;
            char_at_cursor_pos_is_not_cursor = true;
        }

        if (needs_refresh.load())
        {
            needs_refresh.store(false);

            // Ask ncurses to resize according to the new terminal size
            getmaxyx(stdscr, new_rows, new_cols);

            // If you must ensure the curses structures match the new size:
            // You can check if resizing is needed and then call:
            resize_term(new_rows, new_cols);

            // After resizing, clear and redraw everything:
            clear();
            // Redraw from video_memory (up to the new_cols and new_rows,
            // or the original WIDTH and HEIGHT if still valid)
            for (unsigned int y = 0; y < std::min(static_cast<unsigned int>(HEIGHT), new_rows); y++) {
                for (unsigned int x = 0; x < std::min(static_cast<unsigned int>(WIDTH), new_cols); x++) {
                    mvaddch(y, x, video_memory[x][y]);
                }
            }

            refresh();
        }

        if (video_memory_changed.load())
        {
            std::lock_guard lock(memory_access_mutex);

            for (unsigned int y = 0; y < std::min(static_cast<unsigned int>(HEIGHT), new_rows); y++) {
                for (unsigned int x = 0; x < std::min(static_cast<unsigned int>(WIDTH), new_cols); x++) {
                    mvaddch(y, x, video_memory[x][y]);
                }
            }

            curs_set(FALSE);
            refresh();
            video_memory_changed.store(false);
            needs_refresh.store(false);
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
    while (monitor_input_status.load())
    {
        nodelay(stdscr, TRUE);
        int input = getch();
        if (input == KEY_RESIZE) {
            needs_refresh.store(true);
        }
        // Ensure thread-safety of event processor as needed
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_MONITOR_METHOD_NAME)(input);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    monitor_input_exited.store(true);
}

void ui_curses::start_curses()
{
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
}

void sig_handle(int sig)
{
    if (sig == SIGWINCH) {
        needs_refresh.store(true);
    }
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

    signal(SIGWINCH, sig_handle);

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

    if (!char_at_cursor_pos_is_not_cursor) {
        video_memory[cursor_pos.x][cursor_pos.y] = char_at_cursor_position; // write char from backup
    }

    char_at_cursor_position = video_memory[x][y];
    video_memory_changed = true;

    // update position
    cursor_pos.x = x;
    cursor_pos.y = y;
}

cursor_position_t ui_curses::get_cursor()
{
    std::lock_guard lock(memory_access_mutex);
    return { .x = cursor_pos.x, .y = cursor_pos.y };
}

void ui_curses::display_char(int x, int y, int ch)
{
    std::lock_guard lock(memory_access_mutex);

    if (x == cursor_pos.x && y == cursor_pos.y)
    {
        // If we're writing at the cursor position
        if (char_at_cursor_pos_is_not_cursor) {
            video_memory[x][y] = ch;
        } else {
            char_at_cursor_position = ch;
        }
    }
    else
    {
        // If we're writing at a position that is not the cursor position
        video_memory[x][y] = ch;
    }

    video_memory_changed.store(true);
}

void ui_curses::set_cursor_visibility(bool visible)
{
    std::lock_guard lock(memory_access_mutex);
    cursor_visibility = visible;
}
