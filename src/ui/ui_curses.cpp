#include <ui_curses.h>
#include <global.h>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <ncurses.h>

void ui_curses::run(std::atomic<bool>& running, std::atomic<bool>& stopped)
{
    set_thread_name("UI Runner");

    // 1) Initialize ncurses
    initscr();            // Start curses mode
    cbreak();             // Disable line buffering, pass key presses directly
    noecho();             // Do not echo typed characters automatically
    keypad(stdscr, true); // Enable arrow keys, F-keys, etc.

    // Non-blocking getch() so we can do other logic if needed
    // (Alternatively, you could block, but this is common in interactive UIs.)
    nodelay(stdscr, true);
    // ^ Setting this to 'false' means getch() will block.
    //   If you want partial concurrency or periodic checks, use 'true'.

    // 2) Enable mouse events (for scrolling)
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    // 3) Initialize video memory
    {
        std::lock_guard<std::mutex> lock(g_data_mutex);
        init_video_memory();
    }

    // 4) Perform an initial rendering
    {
        std::lock_guard<std::mutex> lock(g_data_mutex);
        render_screen();
    }
    refresh();

    while (running)
    {
        // Wait for a key or mouse event
        int ch = getch();

        // Check if we actually got something
        if (ch == ERR) {
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
            continue;
        }

        // Handle user input
        {
            // 1) Check for mouse events (wheel up/down)
            if (ch == KEY_MOUSE)
            {
                MEVENT me;
                if (getmouse(&me) == OK)
                {
                    // Wheel up -> bstate & BUTTON4_PRESSED
                    // Wheel down -> bstate & BUTTON5_PRESSED
                    if (me.bstate & BUTTON4_PRESSED)
                    {
                        if (offset_y > 0)
                        {
                            std::lock_guard<std::mutex> lock(g_data_mutex);
                            --offset_y;
                            render_screen();
                            refresh();
                        }
                    }
                    else if (me.bstate & BUTTON5_PRESSED)
                    {
                        std::lock_guard<std::mutex> lock(g_data_mutex);
                        ++offset_y;
                        render_screen();
                        refresh();
                    }
                }
            }
            // 2) **Press Ctrl+L to re-render** (Ctrl+L is ASCII 12) (or it just resizes itself)
            else if (ch == 12 || ch == 410)
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                // Just re-render the screen
                render_screen();
                refresh();
            } else {
                GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_MONITOR_METHOD_NAME)(ch);
            }

            if (video_memory_changed)
            {
                std::lock_guard<std::mutex> lock(g_data_mutex);
                render_screen();
                refresh();
                video_memory_changed = false;
            }

            // 3 ms delay
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    // Clean up
    endwin();

    stopped = true;
}

// -----------------------------------------------------
// init_video_memory()
//  Fill the buffer with dots and place a sample message
// -----------------------------------------------------
void ui_curses::init_video_memory()
{
    for (int y = 0; y < V_HEIGHT; y++) {
        for (int x = 0; x < V_WIDTH; x++) {
            video_memory[x][y] = '.';
        }
    }

    // Place a message in the middle
    const auto msg = "(Video Memory Not Initialized)";
    const int msg_len     = static_cast<int>(std::strlen(msg));
    const int mid_x       = (V_WIDTH  - msg_len) / 2;
    constexpr int mid_y       = (V_HEIGHT - 1) / 2;

    for (int i = 0; i < msg_len; i++) {
        video_memory[mid_x + i][mid_y] = static_cast<unsigned char>(msg[i]);
    }
}

// -----------------------------------------------------
// render_screen()
//  Clear the terminal and draw the portion of video_memory
// -----------------------------------------------------
void ui_curses::render_screen()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    recalc_offsets(rows, cols);

    curs_set(FALSE);
    clear();

    // Clip the drawing to what fits on the screen
    const int max_x = (cols >= V_WIDTH)  ? V_WIDTH  : cols;
    const int max_y = (rows >= V_HEIGHT) ? V_HEIGHT : rows;

    for (int y = 0; y < max_y; y++)
    {
        for (int x = 0; x < max_x; x++)
        {
            const int sx = offset_x + x;
            const int sy = offset_y + y;

            if (sx >= 0 && sx < cols && sy >= 0 && sy < rows) {
                mvaddch(sy, sx, video_memory[x][y]);
            }
        }
    }

    const auto [x, y] = current_cursor_position.load();
    const int screen_x = offset_x + x;
    const int screen_y = offset_y + y;
    move(screen_y, screen_x);
    curs_set(TRUE);
}

// -----------------------------------------------------
// recalc_offsets()
//  If the terminal is larger than V_WIDTH/V_HEIGHT, center.
//  Otherwise, clamp so we don't go off screen.
// -----------------------------------------------------
void ui_curses::recalc_offsets(int rows, int cols)
{
    // Horizontal
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

    // Vertical
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

void ui_curses::initialize()
{
    renderer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ui_curses::cleanup()
{
    debug::log("[Curses] Stopping renderer...\n");
    renderer.stop();
    debug::log("[Curses] Shutdown procedure finished!\n");
}

void ui_curses::set_cursor(const int x, const int y)
{
    const decltype(current_cursor_position.load()) pos = {.x = x, .y = y};
    current_cursor_position = pos;
}

cursor_position_t ui_curses::get_cursor()
{
    return current_cursor_position;
}

void ui_curses::display_char(int x, int y, int ch)
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    video_memory[x][y] = ch;
    video_memory_changed = true;
}

void ui_curses::set_cursor_visibility(bool visible)
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    curs_set(visible);
}
