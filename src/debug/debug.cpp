#include <debug.h>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <regex>
#include <cxxabi.h>

#define MAX_STACK_FRAMES 64
#define _RED_     "\033[31m"
#define _GREEN_   "\033[32m"
#define _BLUE_    "\033[34m"
#define _PURPLE_  "\033[35m"
#define _YELLOW_  "\033[33m"
#define _CYAN_    "\033[36m"
#define _CLEAR_   "\033[0m"
#define _BOLD_    "\033[1m"
#define _REGULAR_ "\033[0m"

// helper: obtain stack frame:
struct backtrace_info
{
    std::vector < std::string > backtrace_symbols;
    std::vector < void * > backtrace_frames;
};
backtrace_info obtain_stack_frame();

std::string debug::get_current_date_time()
{
    std::ostringstream ret;

    const auto now = std::chrono::system_clock::now();
    const auto now_as_time_t = std::chrono::system_clock::to_time_t(now);
    const auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    const auto epoch = now_ms.time_since_epoch();
    const auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    const long duration = value.count();
    const long ms = duration % 10000000000000;
    const std::tm* local_time = std::localtime(&now_as_time_t);

    ret << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    ret << '.' << std::setfill('0') << std::setw(13) << ms;

    return ret.str();
}

std::string initialize_error_msg(const std::string& msg, const int _errno, const int code, const bool if_perform_code_backtrace)
{
    std::stringstream err_msg;
    err_msg << "=================================================================\n";
    err_msg << _RED_ _BOLD_
            << "Exception Thrown at "
            << debug::get_current_date_time()
            << " Code=" << code << "(" << ERROR_CODE_TO_STRING[code] << ")"
            << _REGULAR_ << "\n";
    err_msg << _BLUE_ _BOLD_
            << "Error description: error=" << code << ": " << ERROR_CODE_TO_STRING[code] << ": " << msg << "\n";
    err_msg << "System Error: errno=" << _errno << ": " << strerror(_errno) << _REGULAR_ << "\n";

    // Section 1: Backtrace
    // perform backtrace. if you are literally the backtrace function, and wish to throw an error, disable this
    if (if_perform_code_backtrace)
    {
        err_msg << _YELLOW_ _BOLD_ "Backtrace starts here:\n" _REGULAR_;
        auto backtrace = obtain_stack_frame();
        int cur_trace = 0;
        for (const auto & trace : backtrace.backtrace_symbols)
        {
            err_msg << "    " _PURPLE_ "Frame #" << cur_trace
                    << " " << backtrace.backtrace_frames[cur_trace] << _REGULAR_ << "\n";
            // TODO: Better trace info
            err_msg << "    |=>" << trace << "\n";

            cur_trace++;
        }
        err_msg << _YELLOW_ _BOLD_ "Backtrace ends here.\n" _REGULAR_;
    }

    err_msg << "=================================================================\n";

    return err_msg.str();
}

SysdarftBaseError::SysdarftBaseError(const std::string& msg, const int _code, const bool if_perform_code_backtrace)
:
    runtime_error(
        initialize_error_msg(
            msg,
            errno,
            _code,
            if_perform_code_backtrace
        )
    ),
    code(_code),
    cur_errno(errno)
{
}

backtrace_info obtain_stack_frame()
{
    std::vector < std::string > backtrace_symbols_ret;
    std::vector < void * > backtrace_frames_ret;
    void* buffer[MAX_STACK_FRAMES] = {};
    const int frames = backtrace(buffer, MAX_STACK_FRAMES);

    // Get backtrace symbols
    char** symbols = backtrace_symbols(buffer, frames);
    if (symbols == nullptr) {
        throw BacktraceError("backtrace_symbols(buffer, frames) provided an empty symbol table!");
    }

    // Iterate through the stack frames
    for (int i = 1; i < frames; ++i)
    {
        backtrace_symbols_ret.emplace_back(symbols[i]);
        backtrace_frames_ret.emplace_back(buffer[i]);
    }

    free(symbols);
    return backtrace_info {backtrace_symbols_ret, backtrace_frames_ret};
}
