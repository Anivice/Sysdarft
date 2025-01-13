// SysdarftCursesUI.cpp
#include <SysdarftCursesUI.h>

SysdarftCursesUI::SysdarftCursesUI()
    : cursor_x(0), cursor_y(0), offset_x(0), offset_y(0)
{
    // Initialize video memory with spaces
    for (auto & y : video_memory)
    {
        for (char & x : y) {
            x = ' ';
        }
    }
}

void SysdarftCursesUI::initialize()
{
    initscr();            // Start curses mode
    cbreak();             // Disable line buffering
    noecho();             // Don't echo typed characters
    keypad(stdscr, TRUE); // Enable special keys

    recalc_offsets();
    clear();
    curs_set(1);
    render_screen();      // Render initial screen
    is_inited = true;
}

void SysdarftCursesUI::cleanup()
{
    endwin();
    is_inited = false;
}

void SysdarftCursesUI::set_cursor(const int x, const int y)
{
    if (!is_inited) {
        return;
    }

    // Clamp within virtual screen bounds [0,79]x[0,24]
    cursor_x = std::clamp(x, 0, V_WIDTH  - 1);
    cursor_y = std::clamp(y, 0, V_HEIGHT - 1);
    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();
}

void SysdarftCursesUI::set_cursor_visibility(const bool visible)
{
    if (!is_inited) {
        return;
    }

    curs_set(visible ? 1 : 0);
}

void SysdarftCursesUI::teletype(const int x, const int y, const std::string &text)
{
    if (!is_inited) {
        std::cout << text;
        return;
    }

    const int start_x = std::clamp(x, 0, V_WIDTH - 1);
    const int start_y = std::clamp(y, 0, V_HEIGHT - 1);

    int current_x = start_x;
    int current_y = start_y;

    for (const char ch : text)
    {
        // Wrap to next line if end of line is reached
        if (current_x >= V_WIDTH) {
            current_x = 0;
            current_y++;
            if (current_y >= V_HEIGHT) {
                break; // Stop if beyond virtual screen height
            }
        }
        // Store character in video memory
        video_memory[current_y][current_x] = ch;
        current_x++;
    }

    // Update cursor position after printing
    if (current_x >= V_WIDTH) current_x = V_WIDTH - 1;
    if (current_y >= V_HEIGHT) current_y = V_HEIGHT - 1;
    set_cursor(current_x, current_y);
    render_screen();
}

void SysdarftCursesUI::handle_resize()
{
    if (!is_inited) {
        return;
    }

    recalc_offsets();
    render_screen();
    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();
}

void SysdarftCursesUI::recalc_offsets()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Horizontal centering or clamping
    if (cols >= V_WIDTH) {
        offset_x = (cols - V_WIDTH) / 2;
    } else {
        offset_x = 0;
    }

    // Vertical centering or clamping
    if (rows >= V_HEIGHT) {
        offset_y = (rows - V_HEIGHT) / 2;
    } else {
        offset_y = 0;
    }
}

void SysdarftCursesUI::render_screen()
{
    clear();
    // Render video_memory to the screen using current offsets
    for (int y = 0; y < V_HEIGHT; ++y) {
        for (int x = 0; x < V_WIDTH; ++x) {
            mvaddch(offset_y + y, offset_x + x, video_memory[y][x]);
        }
    }
    // Position the cursor in the re-rendered screen
    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();
}

