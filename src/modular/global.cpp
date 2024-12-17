#include <global_event.h>

MsgMap GlobalEventProcessor;
std::mutex GlobalEventMutex;
GlobalConfig_ GlobalConfig;
std::atomic < int > GlobalEventNotifier = 0;
