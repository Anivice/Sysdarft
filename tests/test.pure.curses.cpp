#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <ncurses.h>

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

// The "video memory"
static char video_memory[V_WIDTH][V_HEIGHT];

// Offsets for rendering (where the virtual screen appears in the terminal)
static int offset_x = 0;
static int offset_y = 0;

// A mutex to guard shared data, if desired (thread safety)
static std::mutex g_data_mutex;

// -----------------------------------------------------
// Forward Declarations
// -----------------------------------------------------
void init_video_memory();
void render_screen();
void recalc_offsets(int rows, int cols);

// -----------------------------------------------------
// Main
// -----------------------------------------------------
int main()
{
    // 1) Initialize ncurses
    initscr();            // Start curses mode
    cbreak();             // Disable line buffering, pass key presses directly
    noecho();             // Do not echo typed characters automatically
    keypad(stdscr, true); // Enable arrow keys, F-keys, etc.

    // Non-blocking getch() so we can do other logic if needed
    // (Alternatively, you could block, but this is common in interactive UIs.)
    nodelay(stdscr, false);
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

    bool done = false;
    while (!done) {
        // Wait for a key or mouse event
        int ch = getch();

        // Check if we actually got something
        if (ch == ERR) {
            // If non-blocking and nothing is pressed, we might do other logic here
            // For blocking mode, we won't get ERR unless there's an issue
            continue;
        }

        // Handle user input
        {
            std::lock_guard<std::mutex> lock(g_data_mutex);

            // 1) Quit if 'q'
            if (ch == 'q') {
                done = true;
            }
            // 2) Insert "Modified!" if 'm'
            else if (ch == 'm') {
                const char* text = "Modified!";
                int tx = 5, ty = 10;
                for (int i = 0; i < (int)std::strlen(text); i++) {
                    if (tx + i < V_WIDTH && ty < V_HEIGHT) {
                        video_memory[tx + i][ty] = text[i];
                    }
                }
                render_screen();
                refresh();
            }
            // 3) Check for mouse events (wheel up/down)
            else if (ch == KEY_MOUSE) {
                MEVENT me;
                if (getmouse(&me) == OK) {
                    // Wheel up -> bstate & BUTTON4_PRESSED
                    // Wheel down -> bstate & BUTTON5_PRESSED
                    if (me.bstate & BUTTON4_PRESSED) {
                        if (offset_y > 0) {
                            offset_y--;
                            render_screen();
                            refresh();
                        }
                    }
                    else if (me.bstate & BUTTON5_PRESSED) {
                        offset_y++;
                        render_screen();
                        refresh();
                    }
                }
            }
            // 4) **Press Ctrl+L to re-render** (Ctrl+L is ASCII 12)
            else if (ch == 12) {
                // Just re-render the screen
                render_screen();
                refresh();
            }
            // (Add other key handling as desired)
        }
    }

    // Clean up
    endwin();
    return 0;
}

// -----------------------------------------------------
// init_video_memory()
//  Fill the buffer with dots and place a sample message
// -----------------------------------------------------
void init_video_memory()
{
    for (int y = 0; y < V_HEIGHT; y++) {
        for (int x = 0; x < V_WIDTH; x++) {
            video_memory[x][y] = '.';
        }
    }

    // Place a message in the middle
    const char* msg = "Hello from video_memory! (Press Ctrl+L to re-render)";
    int msg_len     = (int)std::strlen(msg);
    int mid_x       = (V_WIDTH  - msg_len) / 2;
    int mid_y       = (V_HEIGHT - 1) / 2;

    for (int i = 0; i < msg_len; i++) {
        video_memory[mid_x + i][mid_y] = msg[i];
    }
}

// -----------------------------------------------------
// render_screen()
//  Clear the terminal and draw the portion of video_memory
// -----------------------------------------------------
void render_screen()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    recalc_offsets(rows, cols);

    clear();

    // Clip the drawing to what fits on the screen
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

// -----------------------------------------------------
// recalc_offsets()
//  If the terminal is larger than V_WIDTH/V_HEIGHT, center.
//  Otherwise, clamp so we don't go off screen.
// -----------------------------------------------------
void recalc_offsets(int rows, int cols)
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
