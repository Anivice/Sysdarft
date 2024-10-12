/**
 * @file debug.h
 * @brief This file is intended for providing debug functionality for all functions available in this project
 */

#ifndef SYSDARFT_DEBUG_H
#define SYSDARFT_DEBUG_H

#include <string>
#include <iostream>
#include <exception>

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

class sysdarft_error_t : public std::exception
{
private:
    std::string err_info;

public:
    enum error_types_t {
        SUCCESS,
    };

    explicit sysdarft_error_t(error_types_t);
    [[nodiscard]] const char * what() const noexcept override;
};

namespace sysdarft_log
{
    enum console_color_t { RED, GREEN, BLUE, PURPLE, YELLOW, CYAN, CLEAR, BOLD, REGULAR };

    bool is_console_supporting_colored_output();

    // Helper function to output console colors
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

    // General output to stream function (special handling for console_color_t)
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

    // Variadic template function to output multiple arguments
    template < typename StreamType, typename ParamType, typename... Args >
    void output_to_stream(StreamType& ostream, const ParamType& param, const Args&... args)
    {
        output_to_stream(ostream, param);
        (output_to_stream(ostream, args), ...);
    }

    enum log_level_t { LOG_NORMAL, LOG_ERROR };

    // Log function for a single parameter
    template < typename Type >
    void log(log_level_t level, const Type& param)
    {
        auto& ostream = (level == LOG_ERROR) ? std::cerr : std::cout;
        output_to_stream(ostream, param);
    }

    // Log function for multiple parameters
    template<typename Type, typename... Args>
    void log(log_level_t level, const Type& param, const Args&... args)
    {
        auto& ostream = (level == LOG_ERROR) ? std::cerr : std::cout;
        output_to_stream(ostream, param, args...);
    }

}

#endif // SYSDARFT_DEBUG_H
