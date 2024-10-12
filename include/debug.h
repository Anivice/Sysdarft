/**
 * @file debug.h
 * @brief This file is intended for providing debug functionality for all functions available in this project
 */

#ifndef SYSDARFT_DEBUG_H
#define SYSDARFT_DEBUG_H

#include <string>
#include <iostream>
#include <exception>

std::string obtain_stack_frame();

class sysdarft_error_t : std::exception
{
private:
    std::string err_info;

public:
    enum error_types_t {
        SUCCESS,
        NO_SUCH_LOG_LEVEL,
    };

    explicit sysdarft_error_t(error_types_t);
    [[nodiscard]] const char * what() const noexcept override;
};

namespace log
{
    enum log_level_t { LOG_NORMAL, LOG_ERROR };

    template < typename Type >
    void log(log_level_t level, Type param)
    {
        switch (level)
        {
            case LOG_NORMAL:
                std::cout << param << std::flush;
            break;

            case LOG_ERROR:
                std::cerr << param << std::flush;
            break;

            default:
                throw sysdarft_error_t(sysdarft_error_t::NO_SUCH_LOG_LEVEL);
        }
    }

    template<typename Type, typename... Args>
    void log(log_level_t level, Type param, Args... args)
    {
        switch (level)
        {
            case LOG_NORMAL:
                std::cout << param << std::flush;
            break;

            case LOG_ERROR:
                std::cerr << param << std::flush;
            break;

            default:
                throw sysdarft_error_t(sysdarft_error_t::NO_SUCH_LOG_LEVEL);
        }

        log(level, args...);           // Recursively call print for the rest
    }

}

#endif // SYSDARFT_DEBUG_H
