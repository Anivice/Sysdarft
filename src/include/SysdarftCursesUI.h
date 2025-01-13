#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <array>
#include <mutex>
#include <atomic>
#include <SysdarftDebug.h>
#include <GlobalEvents.h>
#include <WorkerThread.h>

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 127;
static constexpr int V_HEIGHT = 31;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI
{
private:
    std::array< std::array<int, V_HEIGHT>, V_WIDTH> video_memory = { ' ' };
    std::atomic<bool> video_memory_changed = false;
    std::mutex g_data_mutex;

    std::atomic<int> offset_x = 0;
    std::atomic<int> offset_y = 0;

    WorkerThread renderer;

    std::atomic<CursorPosition> current_cursor_position = { };

    void run(std::atomic<bool>& running);
    void init_video_memory();
    void monitor_input();
    void render_screen();
    void recalc_offsets(int rows, int cols);

public:
    SysdarftCursesUI() : renderer(this, &SysdarftCursesUI::run) { }

    void cleanup();
    void initialize();
    void set_cursor(int, int);
    CursorPosition get_cursor();
    void display_char(int, int, int);
    void set_cursor_visibility(bool);
};

#endif //UI_CURSES_H
