#include <global.h>
#include <ui_curses.h>

MsgMap GlobalEventProcessor;
std::mutex GlobalEventMutex;
GlobalConfig_ GlobalConfig;
std::atomic < int > GlobalEventNotifier = 0;
ui_curses curses;
