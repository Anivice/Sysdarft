#include <ui_curses.h>
#include <global_event.h>

class input_processor {
public:
    void input_monitor(int key)
    {
        if (key == ERR) {
            return;
        }

        auto cursor_pos = std::any_cast<cursor_position_t>(
            GlobalEventProcessor(UI_INSTANCE_NAME, UI_GET_CURSOR_METHOD_NAME)());
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_DISPLAY_CHAR_METHOD_NAME)(
            cursor_pos.x, cursor_pos.y, key);

        int linear = cursor_pos.y * WIDTH + cursor_pos.x;
        linear++;
        const auto cursor_pos_y = (linear / WIDTH) & 31;
        const auto cursor_pos_x = (linear % WIDTH) & 127;
        cursor_pos = { .x = cursor_pos_x, .y = cursor_pos_y };

        GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_METHOD_NAME)(
            cursor_pos.x, cursor_pos.y);
    }
};

int main()
{
    debug::verbose = true;
    input_processor processor;
    ui_curses curses;

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &processor,
        UI_INPUT_MONITOR_METHOD_NAME, &input_processor::input_monitor);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_CLEANUP_METHOD_NAME, &ui_curses::cleanup);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_INITIALIZE_METHOD_NAME, &ui_curses::initialize);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_METHOD_NAME, &ui_curses::set_cursor);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_GET_CURSOR_METHOD_NAME, &ui_curses::get_cursor);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_DISPLAY_CHAR_METHOD_NAME, &ui_curses::display_char);

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &ui_curses::set_cursor_visibility);

    GlobalEventProcessor(UI_INSTANCE_NAME, UI_INITIALIZE_METHOD_NAME)();
    GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_VISIBILITY_METHOD_NAME)(true);
    sleep(3600);
    GlobalEventProcessor(UI_INSTANCE_NAME, UI_CLEANUP_METHOD_NAME)();
}
