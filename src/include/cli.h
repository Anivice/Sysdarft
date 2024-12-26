#ifndef CLI_H
#define CLI_H

#include <msg_map.h>
#include <string>
#include <mutex>
#include <ui_curses.h>
#include <thread>
#include <worker.h>

class EXPORT Cli {
private:
    void run(std::atomic<bool> &, std::atomic<bool> &);

    std::string last_command;
    std::mutex access_mutex;
    worker_thread worker;

public:
    Cli();
};

#endif //CLI_H
