// backend_async.cpp
#include "shared.h"

// -----------------------------------------------------------------------------
// 6) The actual global object, plus C ABI for module_init / module_exit
static backend _backend;

class input_processor {
public:
    void input_monitor(int key)
    {
        auto cursor_pos = std::any_cast<cursor_position_t>(
            GlobalEventProcessor(UI_INSTANCE_NAME, UI_GET_CURSOR_METHOD_NAME)());
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_DISPLAY_CHAR_METHOD_NAME)
            (cursor_pos.x, cursor_pos.y, key);

        int linear = cursor_pos.y * V_WIDTH + cursor_pos.x;
        linear++;
        const auto cursor_pos_y = (linear / V_WIDTH) & 31;
        const auto cursor_pos_x = (linear % V_WIDTH) & 127;
        cursor_pos = { .x = cursor_pos_x, .y = cursor_pos_y };

        GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_METHOD_NAME)
            (cursor_pos.x, cursor_pos.y);
    }
};

extern "C" {
int EXPORT module_init(void);
void EXPORT module_exit(void);
std::vector<std::string> EXPORT module_dependencies(void);
}

std::vector<std::string> EXPORT module_dependencies(void) {
    return {};
}

int EXPORT module_init(void)
{
    input_processor input_dummy;

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_CLEANUP_METHOD_NAME, &backend::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_INITIALIZE_METHOD_NAME, &backend::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_SET_CURSOR_METHOD_NAME, &backend::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_GET_CURSOR_METHOD_NAME, &backend::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_DISPLAY_CHAR_METHOD_NAME, &backend::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &_backend,
                                          UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &backend::set_cursor_visibility);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &input_dummy,
                                      UI_INPUT_PROCESSOR_METHOD_NAME, &input_processor::input_monitor);
    log("UI instance overridden!\n");
    return 0;
}

void EXPORT module_exit(void)
{
    log("Backend module exited!\n");
}
