#ifndef CPU_H
#define CPU_H

#include <atomic>
#include <functional>


class processor
{
private:
    void trigger_thread();


public:
    void start_triggering();
};

#endif //CPU_H
