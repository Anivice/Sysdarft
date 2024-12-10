#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <string>
#include <exception>

namespace debug
{
    // get current date time (obviously)
    std::string get_current_date_time();

    template < typename ParamType >
    void _log(const ParamType& param)
    {
        std::cout << param << std::flush;
    }

    template < typename ParamType, typename... Args >
    void _log(const ParamType& param, const Args&... args)
    {
        _log(param);
        (_log(args), ...);
    }

    template<typename... Args>
    void log(const Args&... args)
    {
        _log(get_current_date_time(), ": ");
        _log(args...);
    }
}

enum __ERROR_CODES__
{
    SYSDARFT_OK = 0,
    BACKTRACK_FAILED = 1,
};

const std::string ERROR_CODE_TO_STRING[] = {
    "SYSDARFT OK",
    "Backtrace Failed"
};

class SysdarftBaseError : public std::runtime_error
{
protected:
    int code;
    int cur_errno;
public:
    explicit SysdarftBaseError(const std::string& msg, const int _code, const bool if_perform_code_backtrace = true);
};

class BacktraceError : public SysdarftBaseError
{
    public:
    explicit BacktraceError(const std::string & msg) : SysdarftBaseError(
        std::string("Backtrace Failed: ") + msg,
        BACKTRACK_FAILED,
        false)
    {}
};

#endif //DEBUG_H
