#include <debug.h>
#include <iostream>
#include <cstring>

void f3()
{
    sysdarft_log::log(sysdarft_log::LOG_NORMAL, "Normal\n");
    sysdarft_log::log(sysdarft_log::LOG_ERROR, "Error: errno = ", errno, " (", strerror(errno), ')', '\n');
    sysdarft_log::log(sysdarft_log::LOG_NORMAL, sysdarft_log::GREEN, sysdarft_log::BOLD, "Normal\n",
        sysdarft_log::REGULAR);
    throw sysdarft_error_t(sysdarft_error_t::SUCCESS);
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
        sysdarft_log::log(sysdarft_log::LOG_ERROR, e.what());
    }
}
