#ifndef GLOBAL_EVENT_H
#define GLOBAL_EVENT_H

#include <msg_map.h>
#include <mutex>

#define GLOBAL_INSTANCE_NAME "Global"
#define GLOBAL_DESTROY_METHOD_NAME "destroy"

#define INPUT_INSTANCE_NAME "InputProcessor"
#define INPUT_METHOD_NAME "Input"

extern MsgMap GlobalEventProcessor;
extern std::mutex GlobalEventMutex;

#endif //GLOBAL_EVENT_H
