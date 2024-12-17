#ifndef CLI_H
#define CLI_H

#include <msg_map.h>
#include <string>
#include <mutex>
#include <global_event.h>
#include <ui_curses.h>

class EXPORT Cli {
private:
    void run();

    std::string last_command;
    std::mutex access_mutex;
    ui_curses curses;

public:
    Cli();
};

#endif //CLI_H
