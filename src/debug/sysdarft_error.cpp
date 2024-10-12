#include <debug.h>
#include <sstream>
#include <strstream>
#include <cstring>

const char * sysdarft_errors[] = {
    "Successful",
    "No such log level",
};

sysdarft_error_t::sysdarft_error_t(sysdarft_error_t::error_types_t types)
{
    std::ostrstream str;
    str << "Exception Thrown: " << sysdarft_errors[types]
        << std::endl << "Obtained stack frame:\n"
        << obtain_stack_frame() << "\n"
        << "Errno: " << strerror(errno) << "\n";
    err_info = str.str();
}

const char * sysdarft_error_t::what() const noexcept
{
    return err_info.c_str();
}
