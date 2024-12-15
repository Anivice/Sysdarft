#ifndef CLI_H
#define CLI_H

#include <msg_map.h>
#include <string>
#include <mutex>

#define GLOBAL_INSTANCE_NAME "Global"
#define GLOBAL_DESTROY_METHOD_NAME "destroy"

class Cli {
private:
    void run();

    std::string last_command;
    std::mutex access_mutex;

public:
    MsgMap GlobalEventProcessor;

    Cli();
};

#endif //CLI_H
