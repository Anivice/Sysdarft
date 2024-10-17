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
#include <debug.h>

constexpr auto MAX_STACK_FRAMES = 64;

bool sysdarft_log::is_console_supporting_colored_output()
{
    // Check if the output is a terminal (TTY)
    if (!isatty(fileno(stdout)))
    {
        return false;  // Output is not a terminal (probably a pipe)
    }

    // Check if the terminal supports 256 colors
    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        return false;  // TERM environment variable not set
    }

    // Check if the terminal is one of the 256 color types
    std::string term_str = term;
    if (term_str == "xterm-256color" || term_str.find("256color") != std::string::npos)
    {
        return true;  // 256 color support detected
    }

    return false;  // No 256 color support
}

// Demangle the given mangled C++ symbol name
std::string demangled_name(const std::string& mangled_name)
{
    int status = 0;
    std::unique_ptr<char, decltype(&std::free)> demangled(
        abi::__cxa_demangle(mangled_name.c_str(),
            nullptr, nullptr, &status), std::free);

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
    char exe_path[1024] = {};
    if (readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1) == -1) {
        // std::cerr << "readlink failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
        sysdarft_log::output_to_stream(std::cerr, sysdarft_log::RED,
            "readlink failed while obtaining stack frame! errno: ",
            sysdarft_log::PURPLE, strerror(errno), sysdarft_log::CLEAR);
        return "";
    }

    // Get backtrace symbols
    char** symbols = backtrace_symbols(buffer, frames);
    if (symbols == nullptr) {
        // std::cerr << "backtrace_symbols failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
        sysdarft_log::output_to_stream(std::cerr, sysdarft_log::RED,
            "backtrace_symbols failed while obtaining stack frame! errno: ",
            sysdarft_log::PURPLE, strerror(errno), sysdarft_log::CLEAR);
        return "";
    }

    // Iterate through the stack frames
    for (int i = 1; i < frames; ++i)
    {
        // ret << "# " << i << ": " << symbols[i] << std::endl;
        sysdarft_log::output_to_stream(ret, sysdarft_log::YELLOW, sysdarft_log::BOLD, "# ", i,
            sysdarft_log::REGULAR, ": ", sysdarft_log::PURPLE, symbols[i], sysdarft_log::CLEAR, '\n');

        std::string address = get_addr_from_symbol(symbols[i]);

        if (!address.empty())
        {
            // Use addr2line to resolve file name and line number
            std::ostringstream cmd;
            cmd << "addr2line -e " << exe_path << " -f -p " << address;

            FILE* fp = popen(cmd.str().c_str(), "r");
            if (fp == nullptr)
            {
                // std::cerr << "popen failed while obtaining stack frame! errno: " << strerror(errno) << std::endl;
                sysdarft_log::output_to_stream(std::cerr, sysdarft_log::RED,
                    "popen failed while obtaining stack frame! errno: ",
                    sysdarft_log::PURPLE, strerror(errno), sysdarft_log::CLEAR);
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
                    auto format_processed_regex_replace_output = [&](const std::string & input)->std::string
                    {
                        size_t at_pos = input.find(" at ");
                        std::ostringstream ret;
                        if (at_pos != std::string::npos)
                        {
                            // Separate the string into two parts
                            std::string part1 = input.substr(0, at_pos);   // Part before " at "
                            std::string part2 = input.substr(at_pos + 4);        // Part after " at "

                            sysdarft_log::output_to_stream(ret, sysdarft_log::BLUE, sysdarft_log::BOLD,
                                part1, sysdarft_log::REGULAR, sysdarft_log::RED, " at ", sysdarft_log::CYAN,
                                sysdarft_log::BOLD, part2, sysdarft_log::REGULAR);
                        }

                        return ret.str();
                    };

                    std::string unprocessed_name = match[1].str();
                    std::string processed_name = demangled_name(unprocessed_name);
                    // ret << "    " << std::regex_replace(unprocessed_output, regex_pattern, processed_name + " at");
                   sysdarft_log:: output_to_stream(ret, "    ",
                       format_processed_regex_replace_output(
                        std::regex_replace(unprocessed_output, regex_pattern, processed_name + " at")),
                        sysdarft_log::REGULAR);
                }
                else
                {
                    // ret << "    " << addr2line_output;
                    sysdarft_log::output_to_stream(ret, "    ", sysdarft_log::RED, addr2line_output,
                        sysdarft_log::REGULAR);
                }
            }

            pclose(fp);
        }
        else
        {
            // ret << "    (information unavailable)\n";
            sysdarft_log::output_to_stream(ret, sysdarft_log::RED, sysdarft_log::BOLD,
                "    (information unavailable)\n", sysdarft_log::REGULAR);
        }
    }

    free(symbols);
    return ret.str();
}
