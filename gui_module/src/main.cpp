// main.cpp
#include "gui_module_head.h"

// -----------------------------------------------------------------------------
// 6) The actual global object, plus C ABI for module_init / module_exit
static backend backend_;

static class input_processor {
public:
    void input_monitor(int key)
    {
        auto cursor_pos = g_ui_get_cursor();
        g_ui_display_char(cursor_pos.x, cursor_pos.y, key);

        int linear = cursor_pos.y * V_WIDTH + cursor_pos.x;
        linear++;
        const auto cursor_pos_y = (linear / V_WIDTH) & 31;
        const auto cursor_pos_x = (linear % V_WIDTH) & 127;
        cursor_pos = { .x = cursor_pos_x, .y = cursor_pos_y };

        g_ui_set_cursor(cursor_pos.x, cursor_pos.y);
    }
} input_dummy;

extern "C" {
int SYSDARFT_EXPORT_SYMBOL module_init(void);
void SYSDARFT_EXPORT_SYMBOL module_exit(void);
std::vector<std::string> SYSDARFT_EXPORT_SYMBOL module_dependencies(void);
}

std::vector<std::string> SYSDARFT_EXPORT_SYMBOL module_dependencies(void) {
    return {};
}

int SYSDARFT_EXPORT_SYMBOL module_init(void)
{
    g_ui_cleanup_install(backend_, cleanup);
    g_ui_initialize_install(backend_, initialize);
    g_ui_set_cursor_install(backend_, set_cursor);
    g_ui_get_cursor_install(backend_, get_cursor);
    g_ui_display_char_install(backend_, display_char);
    g_ui_set_cur_vsb_install(backend_, set_cursor_visibility);
    g_input_processor_install(input_dummy, input_monitor);
    log("UI instance overridden!\n");
    return 0;
}

void SYSDARFT_EXPORT_SYMBOL module_exit(void)
{
    log("Backend module exited!\n");
}
