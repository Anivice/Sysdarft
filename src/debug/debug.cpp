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
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <fstream>
#include <cctype>
#include <map>

#define MAX_STACK_FRAMES 64

// Structure to hold thread information
struct ThreadInfo
{
    pid_t tid{};
    std::string name;
    char state{};
    unsigned long utime{};    // User mode jiffies
    unsigned long stime{};    // Kernel mode jiffies
    unsigned long vmsize{};   // Virtual memory size in kB
    unsigned long vmrss{};    // Resident Set Size in kB
    unsigned long starttime{}; // Start time in jiffies
};

std::mutex debug::log_mutex;
std::atomic<bool> debug::verbose = false;

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
    err_msg << _CYAN_ "=================================================================" _REGULAR_ "\n";

    // Retrieve current time once to avoid multiple calls
    const std::string current_time = debug::get_current_date_time();

    err_msg << _RED_ << _BOLD_
            << "Exception Thrown at " << current_time
            << _REGULAR_ << "\n";

    auto replace_all = [](std::string & input,
        const std::string & target,
        const std::string & replacement)
    {
        if (target.empty()) return; // Avoid infinite loop if target is empty

        size_t pos = 0;
        while ((pos = input.find(target, pos)) != std::string::npos) {
            input.replace(pos, target.length(), replacement);
            pos += replacement.length(); // Move past the replacement to avoid infinite loop
        }
    };

    std::string processed_msg = msg;
    if (!processed_msg.empty() && processed_msg.back() == '\n') {
        processed_msg.pop_back();
    }
    replace_all(processed_msg, "\n", "\n" _ITALIC_ _GREEN_ ">>> ");

    // Consolidate the error description
    err_msg << ((_errno == 0) ? _GREEN_ : _RED_)
            << _BOLD_
            << "Error description:\n" _REGULAR_ _ITALIC_ _GREEN_ ">>> " << processed_msg << _REGULAR_ "\n"
            << ((_errno == 0) ? _GREEN_ : _RED_) << "System Error: errno=" << _errno << ": " << strerror(_errno) << _REGULAR_ << "\n";

    // Backtrace section
    if (if_perform_code_backtrace && debug::verbose)
    {
        const std::regex pattern(R"(([^\(]+)\(([^\)]*)\) \[([^\]]+)\])");
        std::smatch matches;

        err_msg << _CYAN_ "=================================================================" _REGULAR_ "\n";
        err_msg << _YELLOW_ << _BOLD_ << "Backtrace starts here:\n" << _REGULAR_;
        auto [backtrace_symbols, backtrace_frames] = debug::obtain_stack_frame();

        for (size_t i = 0; i < backtrace_symbols.size(); ++i)
        {
            std::stringstream prefix;

            if (i == 3) {
                prefix << "   " _GREEN_ _BOLD_ "[" _RED_ "Frame #" << i << " " << backtrace_frames[i] << ": ";
            } else {
                prefix << "    " _PURPLE_ "Frame #" << i << " " << backtrace_frames[i] << _REGULAR_ ": ";
            }
            err_msg << prefix.str();
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
                    }
                    else
                    {
                        std::string caller, path;

                        size_t pos = fd_stdout.find('/'); // Find the position of the first '/'
                        if (pos != std::string::npos)
                        {
                            caller = fd_stdout.substr(0, pos - 4 /* delete " at " */);
                            path = fd_stdout.substr(pos);
                        }

                        size_t pos2 = caller.find('('); // Find the position of the first '/'
                        if (pos2 != std::string::npos) {
                            caller = caller.substr(0, pos2);
                        }

                        if (!caller.empty() && !path.empty()) {
                            std::string empty(prefix.str().length() - 9, ' ');
                            err_msg << (i == 3 ? _RED_ : _BLUE_) << caller << (i == 3 ? _GREEN_ "]\n" : "\n")
                                    << (i == 3 ? _RED_ : _BLUE_) << empty << "at " << path << _REGULAR_;
                        } else {
                            err_msg << (i == 3 ? _RED_ : _BLUE_) << fd_stdout << (i == 3 ? _GREEN_ "]" : "") << _REGULAR_;
                        }
                    }
                };

                if (traced_address.empty()) {
                    generate_addr2line_trace_info(traced_runtime_address);
                } else {
                    generate_addr2line_trace_info(traced_address);
                }
            } else {
                err_msg << _RED_ << "No trace information\n" _REGULAR_ "\n";
            }
        }
        err_msg << _YELLOW_ << _BOLD_ << "Backtrace ends here." _REGULAR_ "\n";
    }

    if (debug::verbose) {
        err_msg << _CYAN_ "=================================================================" _REGULAR_ "\n"
                << _BOLD_ _YELLOW_ "Thread Information:" _REGULAR_ "\n"
                << debug::get_verbose_info();
    }

    if (!debug::verbose) {
        err_msg << _CYAN_ "=================================================================" _REGULAR_ "\n"
                << _BLUE_ _BOLD_ "\n"
                << "If you see this message, but the program continues to work,\n"
                << "it means it's in debug mode, and some error handling functions are missing or not implemented.\n"
                << "If you see this message, and the program crashed, then this means it has unhandled BUGs.\n"
                << "You need to enable verbose mode, and refer to the backtrace to find out where is exception occurred.\n"
                << _REGULAR_ << "\n";
    }

    err_msg << _CYAN_ "=================================================================" _REGULAR_ "\n";

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

// Function to check if a string consists solely of digits
bool isDigits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

// Function to parse /proc/self/task/<tid>/stat
bool parseStatFile(pid_t tid, ThreadInfo& info)
{
    std::string statPath = "/proc/self/task/" + std::to_string(tid) + "/stat";
    std::ifstream statFile(statPath);
    if (!statFile.is_open()) {
        std::cerr << "Failed to open " << statPath << std::endl;
        return false;
    }

    std::string line;
    getline(statFile, line);
    statFile.close();

    // Parsing stat file
    // The format is:
    // pid (comm) state ppid ... utime stime ... starttime ...
    // Since comm can contain spaces and is enclosed in parentheses, find the position after the last ')'
    size_t pos1 = line.find('(');
    size_t pos2 = line.rfind(')');
    if (pos1 == std::string::npos || pos2 == std::string::npos || pos2 <= pos1 + 1) {
        std::cerr << "Malformed stat file for TID " << tid << std::endl;
        return false;
    }

    std::string before = line.substr(0, pos1 - 1); // pid
    std::string comm = line.substr(pos1 + 1, pos2 - pos1 - 1); // comm
    std::string after = line.substr(pos2 + 2); // rest of the fields

    std::istringstream iss(after);
    std::vector<std::string> fields;
    std::string field;
    while (iss >> field) {
        fields.push_back(field);
    }

    if (fields.size() < 44) { // Ensure there are enough fields
        std::cerr << "Not enough fields in stat file for TID " << tid << std::endl;
        return false;
    }

    // Populate ThreadInfo
    info.tid = tid;
    info.name = comm;
    info.state = fields[0][0]; // state is the first field after comm
    try {
        info.utime = std::stoul(fields[11]);    // Field 14: utime
        info.stime = std::stoul(fields[12]);    // Field 15: stime
        info.starttime = std::stoul(fields[19]); // Field 22: starttime
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid number in stat file for TID " << tid << std::endl;
        return false;
    } catch (const std::out_of_range& e) {
        std::cerr << "Number out of range in stat file for TID " << tid << std::endl;
        return false;
    }

    return true;
}

// Function to parse /proc/self/task/<tid>/status
bool parseStatusFile(pid_t tid, ThreadInfo& info)
{
    std::string statusPath = "/proc/self/task/" + std::to_string(tid) + "/status";
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        std::cerr << "Failed to open " << statusPath << std::endl;
        return false;
    }

    std::string line;
    std::map<std::string, std::string> statusMap;
    while (getline(statusFile, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            // Remove leading whitespace from value
            size_t valueStart = line.find_first_not_of(" \t", colon + 1);
            std::string value = (valueStart != std::string::npos) ? line.substr(valueStart) : "";
            statusMap[key] = value;
        }
    }
    statusFile.close();

    // Extract VmSize and VmRSS if available
    if (statusMap.find("VmSize") != statusMap.end()) {
        std::istringstream iss(statusMap["VmSize"]);
        iss >> info.vmsize;
    } else {
        info.vmsize = 0;
    }

    if (statusMap.find("VmRSS") != statusMap.end()) {
        std::istringstream iss(statusMap["VmRSS"]);
        iss >> info.vmrss;
    } else {
        info.vmrss = 0;
    }

    return true;
}

// Function to get thread information
bool getThreadInfo(pid_t tid, ThreadInfo& info)
{
    if (!parseStatFile(tid, info)) {
        return false;
    }
    if (!parseStatusFile(tid, info)) {
        return false;
    }
    return true;
}

std::string debug::get_verbose_info()
{
    std::stringstream ret;
    std::string taskPath = "/proc/self/task/";
    DIR* dir = opendir(taskPath.c_str());
    if (dir == nullptr) {
        std::cerr << "Failed to open directory: " << taskPath << std::endl;
        return "";
    }

    std::vector<ThreadInfo> threads;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        // Entries are directories with numeric names (TIDs)
        if (entry->d_type == DT_DIR) {
            std::string name(entry->d_name);
            if (name == "." || name == "..") {
                continue;
            }
            if (!isDigits(name)) {
                continue;
            }
            pid_t tid = std::stoi(name);
            ThreadInfo info;
            if (getThreadInfo(tid, info)) {
                threads.push_back(info);
            }
        }
    }

    closedir(dir);

    // Determine maximum widths for each column
    size_t max_tid = std::string("TID").size();
    size_t max_state = std::string("State").size();
    size_t max_name = std::string("Name").size();
    size_t max_utime = std::string("Utime").size();
    size_t max_stime = std::string("Stime").size();
    size_t max_vmsize = std::string("Vmsize(kB)").size();
    size_t max_vmrss = std::string("Vmrss(kB)").size();
    size_t max_starttime = std::string("Starttime").size();

    for (const auto& thread : threads)
    {
        max_tid = std::max(max_tid, std::to_string(thread.tid).size());
        max_state = std::max(max_state, std::to_string(thread.state).size());
        max_name = std::max(max_name, thread.name.size());
        max_utime = std::max(max_utime, std::to_string(thread.utime).size());
        max_stime = std::max(max_stime, std::to_string(thread.stime).size());
        max_vmsize = std::max(max_vmsize, std::to_string(thread.vmsize).size());
        max_vmrss = std::max(max_vmrss, std::to_string(thread.vmrss).size());
        max_starttime = std::max(max_starttime, std::to_string(thread.starttime).size());
    }

    // Define padding for each column (max length + 3 spaces)
    size_t pad_tid = max_tid + 3;
    size_t pad_state = max_state + 3;
    size_t pad_name = max_name + 3;
    size_t pad_utime = max_utime + 3;
    size_t pad_stime = max_stime + 3;
    size_t pad_vmsize = max_vmsize + 3;
    size_t pad_vmrss = max_vmrss + 3;
    size_t pad_starttime = max_starttime + 3;

    // Function to create a string with a given number of spaces
    auto create_spaces = [](const size_t count) -> std::string {
        return std::string(count, ' ');
    };

    // Function to pad a string to the right with spaces up to a given width
    auto pad_right = [&](const std::string& s, const size_t width) -> std::string
    {
        if (s.size() >= width) {
            return s;
        }
        return s + create_spaces(width - s.size());
    };

    // Header
    ret << _BOLD_ << _YELLOW_ << "Number of threads in current process: "
        << threads.size() << "\n\n" << _REGULAR_;

    ret << _BLUE_ << "Detailed Thread Information:" _REGULAR_ "\n";
    ret << _CYAN_ "-----------------------------------------------" _REGULAR_ "\n";

    // Table Header
    std::stringstream table;
    table   << _RED_    << pad_right("TID", pad_tid)
            << _GREEN_  << pad_right("State", pad_state)
            << _BLUE_   << pad_right("Name", pad_name)
            << _PURPLE_ << pad_right("Utime", pad_utime)
            << _YELLOW_ << pad_right("Stime", pad_stime)
            << _CYAN_   << pad_right("Vmsize(kB)", pad_vmsize)
            << _RED_    << pad_right("Vmrss(kB)", pad_vmrss)
            << _GREEN_  << pad_right("Starttime", pad_starttime)
            << _REGULAR_ "\n";
    ret << table.str();

    // Separator
    ret << _PURPLE_ << std::string(table.str().length() - 45, '-') << _REGULAR_ "\n";

    // Table Rows
    for (const auto& thread : threads)
    {
        ret << _RED_    << pad_right(std::to_string(thread.tid), pad_tid)
            << _GREEN_  << pad_right(std::string(1, thread.state), pad_state)
            << _BLUE_   << pad_right(thread.name.substr(0, 15), pad_name) // Limit name length for readability
            << _PURPLE_ << pad_right(std::to_string(thread.utime), pad_utime)
            << _YELLOW_ << pad_right(std::to_string(thread.stime), pad_stime)
            << _CYAN_   << pad_right(std::to_string(thread.vmsize), pad_vmsize)
            << _RED_    << pad_right(std::to_string(thread.vmrss), pad_vmrss)
            << _GREEN_  << pad_right(std::to_string(thread.starttime), pad_starttime)
            << _REGULAR_ "\n";
    }

    return ret.str();
}
