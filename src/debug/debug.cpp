#include <debug.h>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <sstream>
#include <regex>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <memory>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <chrono>
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

std::mutex debug::log_mutex;

std::string debug::separate_before_slash(const std::string& input)
{
    size_t pos = input.find('/'); // Find the position of the first '/'
    if (pos != std::string::npos) {
        return input.substr(0, pos); // Return the substring before '/'
    }
    return input; // If '/' is not found, return the entire string
}

debug::backtrace_info debug::obtain_stack_frame()
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

std::string debug::get_current_date_time()
{
    using namespace std::chrono;

    // Get current time
    const auto now = system_clock::now();
    const auto now_time_t = system_clock::to_time_t(now);
    const std::tm local_time = *std::localtime(&now_time_t);

    // Calculate milliseconds part
    const auto now_ms = time_point_cast<milliseconds>(now);
    const auto ms = duration_cast<milliseconds>(now_ms - time_point_cast<seconds>(now_ms)).count();

    // Format output
    std::ostringstream ret;
    ret << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms;

    return ret.str();
}

debug::cmd_status debug::_exec_command(const std::string& cmd, const std::vector<std::string>& args)
{
    debug::cmd_status error_status = {
        .fd_stdout = "",
        .fd_stderr = "",
        .exit_status = 1
    };

    int pipe_stdout[2];
    int pipe_stderr[2];
    if (pipe(pipe_stdout) != 0 || pipe(pipe_stderr) != 0) {
        error_status.fd_stderr = "Failed to create pipe for stdout!";
    }

    const pid_t pid = fork();
    if (pid == -1) {
        error_status.fd_stderr = "Failed to fork process!";
    }

    if (pid == 0) {  // Child process
        // Redirect stdout
        dup2(pipe_stdout[1], STDOUT_FILENO);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);

        // Redirect stderr
        dup2(pipe_stderr[1], STDERR_FILENO);
        close(pipe_stderr[0]);
        close(pipe_stderr[1]);

        // Prepare arguments for execvp
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(cmd.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        // Execute the command
        execvp(cmd.c_str(), argv.data());
        exit(EXIT_FAILURE);  // exec never returns unless there is an error
    }

    // Parent process
    close(pipe_stdout[1]);
    close(pipe_stderr[1]);

    cmd_status result;
    char buffer[256];
    ssize_t count;

    // Read stdout
    while ((count = read(pipe_stdout[0], buffer, sizeof(buffer)-1)) > 0) {
        buffer[count] = '\0';
        result.fd_stdout += buffer;
    }
    close(pipe_stdout[0]);

    // Read stderr
    while ((count = read(pipe_stderr[0], buffer, sizeof(buffer)-1)) > 0) {
        buffer[count] = '\0';
        result.fd_stderr += buffer;
    }
    close(pipe_stderr[0]);

    // Wait for the child to finish
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        result.exit_status = WEXITSTATUS(status);
    } else {
        result.exit_status = -1; // Indicates failure to capture exit status
    }

    return result;
}

/**
 * @brief Generates a formatted error message with optional backtrace information.
 *
 * This function creates a detailed error message string based on the provided parameters,
 * which include the error code, system error number (errno), and a descriptive message.
 * Additionally, if enabled, it collects and formats backtrace information to aid in debugging.
 *
 * The generated message includes:
 * - The current timestamp.
 * - The error code and its corresponding string representation.
 * - The system error code (`errno`) and its description.
 * - A backtrace, including frame details and address-to-line mapping via `addr2line`, if enabled.
 *
 * @param msg A descriptive message explaining the error.
 * @param _errno The system error number (errno), typically set when a system call fails.
 * @param if_perform_code_backtrace A boolean flag indicating whether to include backtrace
 *                                  information in the generated message.
 *
 * @return Returns a `std::string` containing the formatted error message, including optional
 *         backtrace details if enabled.
 *
 * Example Usage:
 * @code
 * std::string error_msg = initialize_error_msg(
 *     "An error occurred",
 *     errno,
 *     MY_ERROR_CODE,
 *     true);
 * std::cout << error_msg << std::endl;
 * @endcode
 *
 * @note Backtrace information is collected using `obtain_stack_frame` and `addr2line` to map
 *       addresses to source code locations. Errors in address resolution are also reported
 *       in the generated message.
 */
std::string initialize_error_msg(
    const std::string& msg,
    const int _errno,
    const bool if_perform_code_backtrace)
{
    std::ostringstream err_msg;
    err_msg << "=================================================================\n";

    // Retrieve current time once to avoid multiple calls
    const std::string current_time = debug::get_current_date_time();

    err_msg << _RED_ << _BOLD_
            << "Exception Thrown at " << current_time
            << _REGULAR_ << "\n";

    // Combine error code and errno color settings into a single check
    err_msg << ((_errno == 0) ? _GREEN_ : _RED_);

    // Consolidate the error description
    err_msg << _BOLD_
            << "Error description: " << msg << "\n"
            << "System Error: errno=" << _errno << ": " << strerror(_errno) << _REGULAR_ << "\n";

    // Backtrace section
    if (if_perform_code_backtrace) {
        const std::regex pattern(R"(([^\(]+)\(([^\)]*)\) \[([^\]]+)\])");
        std::smatch matches;

        err_msg << _YELLOW_ << _BOLD_ << "Backtrace starts here (usually meaningful traces starts from #3):\n" << _REGULAR_;
        auto [backtrace_symbols, backtrace_frames] = debug::obtain_stack_frame();

        for (size_t i = 0; i < backtrace_symbols.size(); ++i) {
            err_msg << "    " << _PURPLE_ << "Frame #" << i << " " << backtrace_frames[i] << _REGULAR_ << ": ";
            if (std::regex_search(backtrace_symbols[i], matches, pattern) && matches.size() > 3) {
                const std::string& executable_path = matches[1].str();
                const std::string& traced_address = matches[2].str();
                const std::string& traced_runtime_address = matches[3].str();

                auto generate_addr2line_trace_info = [&](const std::string & address) {
                    auto [fd_stdout, fd_stderr, exit_status] = debug::exec_command(
                        "addr2line",
                        "--demangle",
                        "-f",
                        "-p",
                        "-a",
                        "-e",
                        executable_path,
                        address);
                    if (exit_status != 0) {
                        err_msg << _RED_ << _BOLD_
                                << "\tObtaining backtrace information failed for "
                                << executable_path << " with offset " << address << ": "
                                << fd_stderr << _REGULAR_ << "\n";
                    } else {
                        err_msg << _BLUE_ << fd_stdout << _REGULAR_;
                    }
                };

                if (traced_address.empty()) {
                    generate_addr2line_trace_info(traced_runtime_address);
                } else {
                    generate_addr2line_trace_info(traced_address);
                }
            } else {
                err_msg << _RED_ << "No trace information\n" << _REGULAR_;
            }
        }
        err_msg << _YELLOW_ << _BOLD_ << "Backtrace ends here.\n" << _REGULAR_;
    }

    err_msg << "=================================================================\n";

    return err_msg.str();
}

SysdarftBaseError::SysdarftBaseError(const std::string& msg, const bool if_perform_code_backtrace)
:
    runtime_error(
        initialize_error_msg(
            msg,
            errno,
            if_perform_code_backtrace
        )
    ),
    cur_errno(errno)
{
}
