#include <ui_curses.h>
#include <thread>
#include <chrono>
#include <global_event.h>
#include <csignal>
#include <unistd.h>
#include <termios.h>
#include <atomic>
#include <ncurses.h>

void ui_curses::run()
{
    // Target 30 FPS
    constexpr int targetFPS = 30;
    const auto frameDuration = std::chrono::milliseconds(1000 / targetFPS);
    uint32_t current_frame_within_seconds = 0;
    auto lastFrameTime = std::chrono::steady_clock::now();

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

        if (video_memory_changed.load())
        {
            std::lock_guard lock(memory_access_mutex);

            for (unsigned int y = 0; y < HEIGHT; y++) {
                for (unsigned int x = 0; x < WIDTH; x++) {
                    mvaddch(y, x, video_memory[x][y]);
                }
            }

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

        // Update frame timing
        auto actualFrameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - lastFrameTime);
        lastFrameTime = std::chrono::steady_clock::now();
    }

    running_thread_current_exited.store(true);
}

void ui_curses::monitor_input()
{
    while (monitor_input_status.load())
    {
        nodelay(stdscr, TRUE);
        int input = getch();
        // Ensure thread-safety of event processor as needed
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_MONITOR_METHOD_NAME)(input);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    monitor_input_exited.store(true);
}

void ui_curses::initialize()
{
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
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    signal(SIGWINCH, SIG_IGN);

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
    std::lock_guard lock(memory_access_mutex);

    monitor_input_status.store(false);
    running_thread_current_status.store(false);
    endwin(); // End ncurses mode

    // Wait for threads to exit
    for (uint32_t try_ = 0; try_ < 100; try_++)
    {
        if (running_thread_current_exited.load() && monitor_input_exited.load()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
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

    if (x = cursor_pos.x, y = cursor_pos.y) {
        if (char_at_cursor_pos_is_not_cursor) {
            video_memory[x][y] = ch;
        } else {
            char_at_cursor_position = ch;
        }
    }

    video_memory_changed.store(true);
}

void ui_curses::set_cursor_visibility(bool visible)
{
    std::lock_guard lock(memory_access_mutex);
    cursor_visibility = visible;
}
