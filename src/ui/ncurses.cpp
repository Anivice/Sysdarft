#include <ui_curses.h>
#include <thread>
#include <chrono>
#include <global_event.h>
#include <csignal>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

void ui_curses::run()
{
    // Target 30 FPS
    constexpr int targetFPS = 30;
    const auto frameDuration = std::chrono::milliseconds(1000 / targetFPS);

    auto lastFrameTime = std::chrono::steady_clock::now();

    while (running_thread_current_status.load())
    {
        auto start = std::chrono::steady_clock::now();

        if (video_memory_changed.load())
        {
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

        auto sleepDuration = frameDuration - elapsed;
        if (sleepDuration > std::chrono::milliseconds(0)) {
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
        // Call your event processor - ensure it is thread-safe
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_MONITOR_METHOD_NAME)(input);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    monitor_input_exited.store(true);
}

void ui_curses::initialize()
{
    for (unsigned int y = 0; y < HEIGHT; y++)
    {
        for (unsigned int x = 0; x < WIDTH; x++) {
            video_memory[x][y] = ' ' | COLOR_PAIR(1);
        }
    }

    // Initialize ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    signal(SIGWINCH, SIG_IGN);

    std::thread Worker1(&ui_curses::run, this);
    std::thread Worker2(&ui_curses::monitor_input, this);

    // Detach threads so they run independently
    Worker1.detach();
    Worker2.detach();
}

void ui_curses::cleanup()
{
    monitor_input_status.store(false);
    running_thread_current_status.store(false);
    endwin(); // End ncurses mode

    // Wait for threads to exit
    for (uint32_t try_ = 0; try_ < 100; try_++) {
        if (running_thread_current_exited.load() && monitor_input_exited.load()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void ui_curses::set_cursor(const int x, const int y) {
    move(y, x);
}

cursor_position_t ui_curses::get_cursor()
{
    cursor_position_t ret{};
    getyx(stdscr, ret.y, ret.x);
    return ret;
}

void ui_curses::display_char(int x, int y, int ch)
{
    // Store character in video memory
    video_memory[x][y] = ch;
    video_memory_changed.store(true);
}

void ui_curses::set_cursor_visibility(int visible)
{
    curs_set(visible);
}
