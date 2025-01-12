#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H

#include <SysdarftMessageMap.h>
#include <SysdarftDebug.h>

#define UI_INSTANCE_NAME                        "UI"
#define UI_CLEANUP_METHOD_NAME                  "cleanup"
#define UI_INITIALIZE_METHOD_NAME               "initialize"
#define UI_SET_CURSOR_METHOD_NAME               "set_cursor"
#define UI_TELETYPE_METHOD_NAME                 "teletype"
#define UI_SET_CURSOR_VISIBILITY_METHOD_NAME    "set_cursor_visibility"

extern SYSDARFT_EXPORT_SYMBOL SysdarftMessageMap GlobalEventProcessor;

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
#define g_ui_set_cur_vsb(vsb) GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_VISIBILITY_METHOD_NAME)(vsb)
#define g_ui_set_cur_vsb_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_SET_CURSOR_VISIBILITY_METHOD_NAME, method)
#define g_ui_teletype(x, y, ch) GlobalEventProcessor(UI_INSTANCE_NAME, UI_TELETYPE_METHOD_NAME)(x, y, ch)
#define g_ui_teletype_install(instance, method) _g_method_install(UI_INSTANCE_NAME, instance, UI_TELETYPE_METHOD_NAME, method)

#endif //GLOBAL_EVENT_H
