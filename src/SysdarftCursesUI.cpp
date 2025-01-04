#include <thread>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <SysdarftCursesUI.h>
#include <GlobalEvents.h>

// -----------------------------------------------------
// init_video_memory()
//  Fill the buffer with dots and place a sample message
// -----------------------------------------------------
void SysdarftCursesUI::init_video_memory() const
{
    // Initialize all positions with a dot
    std::fill_n(video_memory, V_WIDTH * V_HEIGHT, '.');

    // Place a message in the middle
    const auto msg = "(Video Memory Initialized)";
    const int msg_len = static_cast<int>(std::strlen(msg));
    int mid_x = (V_WIDTH - msg_len) / 2;
    int mid_y = (V_HEIGHT - 1) / 2;

    // Ensure mid_x and mid_y are within bounds
    mid_x = std::max(0, std::min(mid_x, V_WIDTH - msg_len));
    mid_y = std::max(0, std::min(mid_y, V_HEIGHT - 1));

    // Calculate the starting index for the message
    const int start_index = mid_y * V_WIDTH + mid_x;

    // Insert the message into video_memory
    std::memcpy(&video_memory[start_index], msg, msg_len);
}

// -----------------------------------------------------
// recalc_offsets()
//  If the terminal is larger than V_WIDTH/V_HEIGHT, center.
//  Otherwise, clamp so we don't go off screen.
// -----------------------------------------------------
void SysdarftCursesUI::recalc_offsets(const int rows, const int cols)
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

// -----------------------------------------------------
// render_screen()
//  Clear the terminal and draw the portion of video_memory
// -----------------------------------------------------
void SysdarftCursesUI::render_screen()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Recalculate offsets based on current terminal size
    recalc_offsets(rows, cols);

    // Hide the cursor during rendering for performance
    curs_set(FALSE);
    clear();

    // Determine the drawable width and height based on terminal size and video_memory offsets
    int drawable_width  = std::min(cols - offset_x, V_WIDTH);
    int drawable_height = std::min(rows - offset_y, V_HEIGHT);

    // Ensure drawable dimensions are non-negative
    drawable_width  = std::max(drawable_width, 0);
    drawable_height = std::max(drawable_height, 0);

    // Iterate over each row and column within the drawable area
    for (int y = 0; y < drawable_height; y++)
    {
        for (int x = 0; x < drawable_width; x++)
        {
            // Calculate the source position in video_memory
            const int screen_x = offset_x + x;
            const int screen_y = offset_y + y;

            // Ensure the index is within video_memory bounds
            if (const int index = y * V_WIDTH + x;
                index >= 0 && index < V_WIDTH * V_HEIGHT)
            {
                mvaddch(screen_y, screen_x, video_memory[index]);
            }
        }
    }

    // Update the cursor position relative to the current offsets
    const auto [cursor_x, cursor_y] = current_cursor_position;
    const int screen_x = cursor_x - offset_x;

    // Ensure the cursor is within the drawable area
    if (const int screen_y = cursor_y - offset_y;
        screen_x >= 0 && screen_x < drawable_width &&
        screen_y >= 0 && screen_y < drawable_height)
    {
        move(screen_y, screen_x);
    }

    // Show the cursor again
    curs_set(TRUE);
}

// -----------------------------------------------------
// initialize()
//  Initialize ncurses and set up the initial state
// -----------------------------------------------------
void SysdarftCursesUI::initialize()
{
    // 1) Initialize ncurses
    initscr();                  // Start curses mode
    cbreak();                   // Disable line buffering, pass key presses directly
    noecho();                   // Do not echo typed characters automatically
    keypad(stdscr, TRUE);       // Enable arrow keys, F-keys, etc.

    // 2) Enable mouse events (for scrolling)
    mousemask(ALL_MOUSE_EVENTS, nullptr);

    // 3) Initialize video memory
    {
        std::lock_guard<std::mutex> lock(g_data_mutex);
        init_video_memory();
        render_screen();
    }
    refresh();
}

// -----------------------------------------------------
// cleanup()
//  Shutdown ncurses and perform cleanup
// -----------------------------------------------------
void SysdarftCursesUI::cleanup()
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    endwin();
    log("[Curses] Shutdown procedure finished!\n");
}

// -----------------------------------------------------
// set_cursor()
//  Set the cursor position within video_memory
// -----------------------------------------------------
void SysdarftCursesUI::set_cursor(const int x, const int y)
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    current_cursor_position.x = std::max(0, std::min(x, V_WIDTH - 1));
    current_cursor_position.y = std::max(0, std::min(y, V_HEIGHT - 1));
}

// -----------------------------------------------------
// get_cursor()
//  Get the current cursor position within video_memory
// -----------------------------------------------------
CursorPosition SysdarftCursesUI::get_cursor()
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    return current_cursor_position;
}

// -----------------------------------------------------
// set_cursor_visibility()
//  Show or hide the cursor
// -----------------------------------------------------
void SysdarftCursesUI::set_cursor_visibility(const bool visible)
{
    std::lock_guard<std::mutex> lock(g_data_mutex);
    curs_set(visible ? 1 : 0);
}

// -----------------------------------------------------
// commit_changes()
//  Commit changes by re-rendering the screen and updating the cursor
// -----------------------------------------------------
void SysdarftCursesUI::commit_changes()
{
    std::lock_guard<std::mutex> lock(g_data_mutex);

    // Hide cursor temporarily to prevent flickering
    curs_set(FALSE);

    // Re-render the screen with updated video_memory
    render_screen();
    refresh();

    // Calculate screen position for cursor based on current offsets
    const auto [x, y] = current_cursor_position;
    const int screen_x = x - offset_x;
    const int screen_y = y - offset_y;

    // Ensure the cursor is within the drawable area before moving
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int drawable_width  = std::min(cols, V_WIDTH  - offset_x);
    int drawable_height = std::min(rows, V_HEIGHT - offset_y);
    drawable_width  = std::max(drawable_width, 0);
    drawable_height = std::max(drawable_height, 0);

    if (screen_x >= 0 && screen_x < drawable_width &&
        screen_y >= 0 && screen_y < drawable_height)
    {
        move(screen_y, screen_x);
    }

    // Show cursor again
    curs_set(TRUE);
}
