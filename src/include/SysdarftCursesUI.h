#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <ncurses.h>
#include <string>
#include <SysdarftDebug.h>

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI {
public:
    SysdarftCursesUI();
    void initialize();
    void cleanup();
    void set_cursor(int x, int y);
    void set_cursor_visibility(bool visible);
    void teletype(char text);
    void newline();
    void handle_resize();

protected:
    int cursor_x;
    int cursor_y;

private:
    int offset_x;
    int offset_y;
    char video_memory[V_HEIGHT][V_WIDTH]{};
    bool is_inited = false;

    void recalc_offsets();
    void render_screen();
};

#endif // UI_CURSES_H
