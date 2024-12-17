#ifndef CLI_H
#define CLI_H

#include <msg_map.h>
#include <string>
#include <mutex>
#include <ui_curses.h>
#include <thread>

class EXPORT Cli {
private:
    void run();

    std::string last_command;
    std::mutex access_mutex;
    ui_curses curses;
    std::thread CliWorkThread;
public:
    Cli();
};

#endif //CLI_H
