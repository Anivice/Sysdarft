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
#define ADDR2LINE_CMD_SIZE 256

// Demangle the given mangled C++ symbol name
std::string demangled_name(const std::string& mangled_name)
{
    int status = 0;
    std::unique_ptr<char, decltype(&std::free)> demangled(
        abi::__cxa_demangle(mangled_name.c_str(), nullptr, nullptr, &status), std::free);

    return (status == 0 && demangled) ? std::string(demangled.get()) : mangled_name;
}

// Extract address from the backtrace symbol string
std::string get_addr_from_symbol(const std::string& str)
{
    std::regex re(R"(\(\+0x([0-9a-fA-F]+)\))");
    std::smatch match;
    if (std::regex_search(str, match, re)) {
        return match[1]; // Directly return the matched group
    }

    return "";
}

// Generate the stack trace, resolve symbols and demangle function names
std::string obtain_stack_frame()
{
    std::ostringstream ret;
    void* buffer[MAX_STACK_FRAMES] = {};
    const int frames = backtrace(buffer, MAX_STACK_FRAMES);

    // Get the executable's path
    char exe_path[4096] = {};
    if (readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1) == -1) {
        std::cerr << "readlink failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
        return "";
    }

    // Get backtrace symbols
    char** symbols = backtrace_symbols(buffer, frames);
    if (symbols == nullptr) {
        std::cerr << "backtrace_symbols failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
        return "";
    }

    // Iterate through the stack frames
    for (int i = 1; i < frames; ++i)
    {
        ret << "# " << i << ": " << symbols[i] << std::endl;

        std::string address = get_addr_from_symbol(symbols[i]);

        if (!address.empty())
        {
            // Use addr2line to resolve file name and line number
            char cmd[ADDR2LINE_CMD_SIZE];
            snprintf(cmd, sizeof(cmd), "addr2line -e %s -f -p %s", exe_path, address.c_str());

            FILE* fp = popen(cmd, "r");
            if (fp == nullptr)
            {
                std::cerr << "popen failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
                continue;
            }

            char addr2line_output[1024] = {};
            if (fgets(addr2line_output, sizeof(addr2line_output), fp) != nullptr)
            {
                std::string unprocessed_output = addr2line_output;
                std::regex regex_pattern("^(.*)\\sat");
                std::smatch match;

                if (std::regex_search(unprocessed_output, match, regex_pattern))
                {
                    std::string unprocessed_name = match[1].str();
                    std::string processed_name = demangled_name(unprocessed_name);
                    ret << "    " << std::regex_replace(unprocessed_output, regex_pattern, processed_name + " at");
                }
                else
                {
                    ret << "    " << addr2line_output;
                }
            }

            pclose(fp);
        }
        else
        {
            ret << "    (information unavailable)\n";
        }
    }

    free(symbols);
    return ret.str();
}
