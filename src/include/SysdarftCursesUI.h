#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <ncurses.h>
#include <mutex>
#include <atomic>
#include <SysdarftDebug.h>
#include <GlobalEvents.h>
#include <WorkerThread.h>

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI
{
private:
    std::mutex g_data_mutex;
    char * video_memory = nullptr;
    std::atomic<int> offset_x = 0;
    std::atomic<int> offset_y = 0;
    CursorPosition current_cursor_position { };

    void init_video_memory() const;
    void monitor_input();
    void render_screen();
    void recalc_offsets(int rows, int cols);

public:
    void cleanup();
    void initialize();
    void set_cursor(int, int);
    CursorPosition get_cursor();
    void commit_changes();
    void set_cursor_visibility(bool);
    void register_vm(char * _buffer) {
        video_memory = _buffer;
    }

    int get_input() {
        return getch();
    }
};

#endif //UI_CURSES_H
