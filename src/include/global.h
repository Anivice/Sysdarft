#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H

#include <atomic>
#include <msg_map.h>
#include <mutex>
#include <vector>
#include <string>
#include <config.h>

#define GLOBAL_INSTANCE_NAME            "Global"
#define GLOBAL_DESTROY_METHOD_NAME      "destroy"
#define GLOBAL_INPUT_METHOD_NAME        "Input"
#define GLOBAL_GET_CONFIG_METHOD_NAME   "get_config"
#define GLOBAL_SET_CONFIG_METHOD_NAME   "set_config"

#define UI_INSTANCE_NAME                        "UI"
#define UI_CLEANUP_METHOD_NAME                  "cleanup"
#define UI_INITIALIZE_METHOD_NAME               "initialize"
#define UI_SET_CURSOR_METHOD_NAME               "set_cursor"
#define UI_GET_CURSOR_METHOD_NAME               "get_cursor"
#define UI_DISPLAY_CHAR_METHOD_NAME             "display_char"
#define UI_SET_CURSOR_VISIBILITY_METHOD_NAME    "set_cursor_visibility"
#define UI_INPUT_MONITOR_METHOD_NAME            "input_monitor"

#define GLOBAL_QUIT_EVENT 0x7C00

extern EXPORT MsgMap GlobalEventProcessor;
extern EXPORT std::atomic < int > GlobalEventNotifier;

extern class EXPORT GlobalConfig_ {
private:
    std::mutex mutex_;
    config_t GlobalConfigVector;

public:
    config_t get_config() { std::lock_guard lock(mutex_); return GlobalConfigVector; }
    void set_config(const config_t & config) { std::lock_guard lock(mutex_); GlobalConfigVector = config; }
} GlobalConfig;

#endif //GLOBAL_EVENT_H
