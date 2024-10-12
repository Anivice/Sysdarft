#include <debug.h>
#include <iostream>
#include <cstring>

void f3()
{
    log::log(log::LOG_NORMAL, "Normal\n");
    log::log(log::LOG_ERROR, "Error: errno = ", errno, " (", strerror(errno), ')', '\n');
    log::log(log::LOG_NORMAL, "Normal\n");
    log::log((log::log_level_t)3, "ss");
}

void f2()
{
    f3();
}

void f1()
{
    f2();
}

int main()
{
    try {
        f1();
    } catch (sysdarft_error_t & e) {
        log::log(log::LOG_ERROR, e.what());
    }
}