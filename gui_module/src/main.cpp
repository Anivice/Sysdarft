// backend_async.cpp
#include "shared.h"

// -----------------------------------------------------------------------------
// 6) The actual global object, plus C ABI for module_init / module_exit
static backend dummy;

extern "C" {
int EXPORT module_init(void);
void EXPORT module_exit(void);
std::vector<std::string> EXPORT module_dependencies(void);
}

std::vector<std::string> EXPORT module_dependencies(void) {
    return {};
}

int EXPORT module_init(void) {
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_CLEANUP_METHOD_NAME, &backend::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_INITIALIZE_METHOD_NAME, &backend::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_SET_CURSOR_METHOD_NAME, &backend::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_GET_CURSOR_METHOD_NAME, &backend::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_DISPLAY_CHAR_METHOD_NAME, &backend::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
                                          UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &backend::set_cursor_visibility);
    debug::log("UI instance overridden!\n");
    return 0;
}

void EXPORT module_exit(void) {
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
    debug::log("UI instance restored!\n");
    debug::log("Backend exited!\n");
}
