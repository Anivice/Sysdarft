#include <debug.h>
#include <sstream>
#include <strstream>
#include <cstring>

const char * sysdarft_errors[] = {
    "Success",
    "Screen service loop failed to start",
    "Screen service loop failed to stop",
    "FUSE service failed to start",
    "FUSE service failed to stop",
    "Filesystem create directories failed",
    "Failed to obtain dynamic libraries",
};

inline std::string init_error_msg(const sysdarft_error_t::error_types_t types)
{
    std::ostringstream str;
    sysdarft_log::output_to_stream(str, sysdarft_log::GREEN, sysdarft_log::BOLD, "Exception Thrown: ",
        sysdarft_log::RED, sysdarft_errors[types], '\n', sysdarft_log::REGULAR,
        sysdarft_log::GREEN, "Obtained stack frame:\n", sysdarft_log::REGULAR,
        obtain_stack_frame(), "\n",
        sysdarft_log::RED, sysdarft_log::BOLD, "Errno: ", strerror(errno), "\n",sysdarft_log::REGULAR);
    return str.str();
}

sysdarft_error_t::sysdarft_error_t(const sysdarft_error_t::error_types_t types)
    : std::runtime_error(init_error_msg(types))
{
    err_info = std::runtime_error::what();
}

const char * sysdarft_error_t::what() const noexcept
{
    return err_info.c_str();
}
