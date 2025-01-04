#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H

#include <SysdarftMessageMap.h>
#include <SysdarftDebug.h>

#define UI_INSTANCE_NAME                        "UI"
#define UI_CLEANUP_METHOD_NAME                  "cleanup"
#define UI_INITIALIZE_METHOD_NAME               "initialize"
#define UI_SET_CURSOR_METHOD_NAME               "set_cursor"
#define UI_GET_CURSOR_METHOD_NAME               "get_cursor"
#define UI_DISPLAY_CHAR_METHOD_NAME             "display_char"
#define UI_SET_CURSOR_VISIBILITY_METHOD_NAME    "set_cursor_visibility"
#define UI_INPUT_PROCESSOR_METHOD_NAME          "input_processor"
#define UI_GET_INPUT                            "get_input"

extern SYSDARFT_EXPORT_SYMBOL SysdarftMessageMap GlobalEventProcessor;

struct SYSDARFT_EXPORT_SYMBOL CursorPosition
{
    int x;
    int y;
    bool operator==(const CursorPosition & cursor_pos) const {
        return x == cursor_pos.x && y == cursor_pos.y;
    }
};


#define _g_method_install(instance_name, instance, method_name, method) \
    GlobalEventProcessor.install_instance(                              \
                instance_name, &instance,                               \
                method_name, &decltype(instance)::method)

#define g_ui_cleanup() GlobalEventProcessor(UI_INSTANCE_NAME, UI_CLEANUP_METHOD_NAME)()
#define g_ui_cleanup_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_CLEANUP_METHOD_NAME, method)
#define g_ui_initialize() GlobalEventProcessor(UI_INSTANCE_NAME, UI_INITIALIZE_METHOD_NAME)()
#define g_ui_initialize_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_INITIALIZE_METHOD_NAME, method)
#define g_ui_set_cursor(x, y) GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_METHOD_NAME)(x, y)
#define g_ui_set_cursor_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_SET_CURSOR_METHOD_NAME, method)
#define g_ui_get_cursor() std::any_cast<CursorPosition>(GlobalEventProcessor(UI_INSTANCE_NAME, UI_GET_CURSOR_METHOD_NAME)())
#define g_ui_get_cursor_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_GET_CURSOR_METHOD_NAME, method)
#define g_ui_display_char(x, y, ch) GlobalEventProcessor(UI_INSTANCE_NAME, UI_DISPLAY_CHAR_METHOD_NAME)(x, y, ch)
#define g_ui_display_char_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_DISPLAY_CHAR_METHOD_NAME, method)
#define g_ui_set_cur_vsb(vsb) GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_VISIBILITY_METHOD_NAME)(vsb)
#define g_ui_set_cur_vsb_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_SET_CURSOR_VISIBILITY_METHOD_NAME, method)
#define g_input_processor(ch) GlobalEventProcessor(UI_INSTANCE_NAME, UI_INPUT_PROCESSOR_METHOD_NAME)(ch)
#define g_input_processor_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_INPUT_PROCESSOR_METHOD_NAME, method)
#define g_get_input() std::any_cast<int>(GlobalEventProcessor(UI_INSTANCE_NAME, UI_GET_INPUT)())
#define g_get_input_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_GET_INPUT, method)

#endif //GLOBAL_EVENT_H
