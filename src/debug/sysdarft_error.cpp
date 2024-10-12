#include <debug.h>
#include <sstream>
#include <strstream>
#include <cstring>

const char * sysdarft_errors[] = {
    "Successful",
};

sysdarft_error_t::sysdarft_error_t(sysdarft_error_t::error_types_t types)
{
    std::ostringstream str;
    sysdarft_log::output_to_stream(str, sysdarft_log::GREEN, sysdarft_log::BOLD, "Exception Thrown: ",
        sysdarft_log::RED, sysdarft_errors[types], sysdarft_log::REGULAR,
        '\n', sysdarft_log::GREEN, "Obtained stack frame:\n", sysdarft_log::REGULAR,
        obtain_stack_frame(), "\n",
        sysdarft_log::RED, sysdarft_log::BOLD, "Errno: ", strerror(errno), "\n", sysdarft_log::REGULAR);
    err_info = str.str();
}

const char * sysdarft_error_t::what() const noexcept
{
    return err_info.c_str();
}
