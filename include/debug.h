#ifndef SYSDARFT_DEBUG_H
#define SYSDARFT_DEBUG_H

#include <string>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <execinfo.h>
#include <unistd.h>
#include <cstring>
#include <regex>
#include <cxxabi.h>

#define _RED_     "\033[31m"
#define _GREEN_   "\033[32m"
#define _BLUE_    "\033[34m"
#define _PURPLE_  "\033[35m"
#define _YELLOW_  "\033[33m"
#define _CYAN_    "\033[36m"
#define _CLEAR_   "\033[0m"
#define _BOLD_    "\033[1m"
#define _REGULAR_ "\033[0m"

std::string obtain_stack_frame();
std::string demangled_name(const std::string&);
std::string get_addr_from_symbol(const std::string&);

class sysdarft_error_t : public std::runtime_error
{
private:
    std::string err_info;

public:
    enum error_types_t {
        SUCCESS,
        SCREEN_SERVICE_LOOP_EXECUTION_FAILED,
        SCREEN_SERVICE_LOOP_FAILED_TO_STOP,
        FUSE_SERVICE_FAILED_TO_START,
        FUSE_SERVICE_FAILED_TO_STOP,
        FILESYSTEM_CREATE_DIRECTORIES_FAILED,
        CANNOT_OBTAIN_DYNAMIC_LIBRARIES,
        INVALID_WAV_FILE,
        PULSEAUDIO_CONNECTION_FAILED,
        PULSEAUDIO_BUFFER_APPEND_FAILED,
        PULSEAUDIO_BUFFER_DRAIN_FAILED,
        PULSEAUDIO_BUFFER_FLUSH_FAILED,
    };

    explicit sysdarft_error_t(error_types_t);
    [[nodiscard]] const char * what() const noexcept override;
};

namespace sysdarft_log
{
    enum console_color_t { RED, GREEN, BLUE, PURPLE, YELLOW, CYAN, CLEAR, BOLD, REGULAR };

    bool is_console_supporting_colored_output();

    template <typename StreamType>
    void output_console_color(StreamType& ostream, console_color_t color)
    {
        if (!is_console_supporting_colored_output()) return;

        switch (color)
        {
            case RED: ostream << _RED_; break;
            case GREEN: ostream << _GREEN_; break;
            case BLUE: ostream << _BLUE_; break;
            case PURPLE: ostream << _PURPLE_; break;
            case YELLOW: ostream << _YELLOW_; break;
            case CYAN: ostream << _CYAN_; break;
            case CLEAR: ostream << _CLEAR_; break;
            case BOLD: ostream << _BOLD_; break;
            case REGULAR: ostream << _REGULAR_; break;
        }
    }

    template < typename StreamType, typename ParamType >
    void output_to_stream(StreamType& ostream, const ParamType& param)
    {
        if constexpr (std::is_same_v<ParamType, console_color_t>)
        {
            output_console_color(ostream, param);
        }
        else
        {
            ostream << param << std::flush;
        }
    }

    template < typename StreamType, typename ParamType, typename... Args >
    void output_to_stream(StreamType& ostream, const ParamType& param, const Args&... args)
    {
        output_to_stream(ostream, param);
        (output_to_stream(ostream, args), ...);
    }

    enum log_level_t { LOG_NORMAL, LOG_ERROR };

    inline auto get_current_date_time = []()->std::string
    {
        std::ostringstream ret;
        const auto now = std::chrono::system_clock::now();
        const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        const std::tm* local_time = std::localtime(&now_time);
        ret << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
        return ret.str();
    };

    // Helper function to get stack frame and invocation line
    inline std::string get_invocation_line()
    {
        constexpr int MAX_STACK_FRAMES = 4;
        void* buffer[MAX_STACK_FRAMES] = {};
        const int frames = backtrace(buffer, MAX_STACK_FRAMES);
        std::string invocation_line = "(unknown line of code)";

        char exe_path[1024] = {};
        if (readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1) == -1) {
            sysdarft_log::output_to_stream(std::cerr, sysdarft_log::RED,
                                           "readlink failed while obtaining stack frame! errno: ",
                                           sysdarft_log::PURPLE, strerror(errno), sysdarft_log::CLEAR);
        }

        char** symbols = backtrace_symbols(buffer, frames);
        if (symbols == nullptr) {
            sysdarft_log::output_to_stream(std::cerr, sysdarft_log::RED,
                                           "backtrace_symbols failed while obtaining stack frame! errno: ",
                                           sysdarft_log::PURPLE, strerror(errno), sysdarft_log::CLEAR);
        }

        if (frames >= 4)
        {
            std::string address = get_addr_from_symbol(symbols[3]);
            if (!address.empty())
            {
                std::ostringstream cmd;
                cmd << "addr2line -e " << exe_path << " -f -p " << address;

                FILE* fp = popen(cmd.str().c_str(), "r");
                if (fp != nullptr)
                {
                    char addr2line_output[1024] = {};
                    if (fgets(addr2line_output, sizeof(addr2line_output), fp) != nullptr)
                    {
                        std::string input = addr2line_output;
                        size_t at_pos = input.find(" at ");
                        if (at_pos != std::string::npos)
                        {
                            invocation_line = input.substr(at_pos + 4);
                        }

                        pclose(fp);
                    }
                }
            }

            free(symbols);
        }

        return invocation_line;
    }

    // Log function for variadic parameters
    template<typename... Args>
    void log(const log_level_t level, const Args&... args)
    {
        std::string invocation_line = get_invocation_line();
        auto& ostream = (level == LOG_ERROR) ? std::cerr : std::cout;
        output_to_stream(ostream, BLUE, BOLD, "[", get_current_date_time(), "]: From ",
                         GREEN, invocation_line, REGULAR);
        output_to_stream(ostream, " ==> ", args...);
    }
}

#endif // SYSDARFT_DEBUG_H
