#include <ui_curses.h>
#include <global_event.h>

class input_processor {
public:
    void input_monitor(int key) {
        if (key != ERR) {
            int ch = 'S';
            GlobalEventProcessor.invoke_instance(
                UI_INSTANCE_NAME,
                UI_DISPLAY_CHAR_METHOD_NAME,
                { 1, 3, ch });
        }
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

    curses.initialize();
    pause();
    curses.cleanup();
}
