#include <algorithm>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <execinfo.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <cstdint>
#include <fcntl.h>
#include <SysdarftDebug.h>

/* Since pipes are unidirectional, we need three pipes:
   1. Parent writes to child's stdin
   2. Child writes to parent's stdout
   3. Child writes to parent's stderr
*/

#define NUM_PIPES           3

#define PARENT_WRITE_PIPE   0
#define PARENT_READ_PIPE    1
#define PARENT_ERR_PIPE     2

int pipes[NUM_PIPES][2];

/* Always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

#define PARENT_READ_FD   ( pipes[PARENT_READ_PIPE][READ_FD]   )
#define PARENT_WRITE_FD  ( pipes[PARENT_WRITE_PIPE][WRITE_FD] )
#define PARENT_ERR_FD    ( pipes[PARENT_ERR_PIPE][READ_FD]    )

#define CHILD_READ_FD    ( pipes[PARENT_WRITE_PIPE][READ_FD]  )
#define CHILD_WRITE_FD   ( pipes[PARENT_READ_PIPE][WRITE_FD]  )
#define CHILD_ERR_FD     ( pipes[PARENT_ERR_PIPE][WRITE_FD]   )

#define MAX_STACK_FRAMES 64

// Structure to hold thread information
struct ThreadInfo {
    pid_t tid {};
    std::string name;
    char state {};
    unsigned long utime {}; // User mode jiffies
    unsigned long stime {}; // Kernel mode jiffies
    unsigned long vmsize {}; // Virtual memory size in kB
    unsigned long vmrss {}; // Resident Set Size in kB
    unsigned long starttime {}; // Start time in jiffies
};

std::mutex debug::log_mutex;
std::atomic<bool> debug::verbose = false;

std::string debug::separate_before_slash(const std::string& input)
{
    size_t pos = input.find('/');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input;
}

debug::backtrace_info debug::obtain_stack_frame()
{
    backtrace_info result;
    void* buffer[MAX_STACK_FRAMES] = {};
    const int frames = backtrace(buffer, MAX_STACK_FRAMES);

    char** symbols = backtrace_symbols(buffer, frames);
    if (symbols == nullptr) {
        return debug::backtrace_info {};
    }

    for (int i = 1; i < frames; ++i) {
        result.emplace_back(symbols[i], buffer[i]);
    }

    free(symbols);
    return result;
}

std::string debug::get_current_date_time()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const auto now_time_t = system_clock::to_time_t(now);
    const std::tm local_time = *std::localtime(&now_time_t);

    const auto now_ms = time_point_cast<milliseconds>(now);
    const auto ms
        = duration_cast<milliseconds>(now_ms - time_point_cast<seconds>(now_ms))
              .count();

    std::ostringstream ret;
    ret << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setfill('0') << std::setw(3) << ms;

    return ret.str();
}


// Helper function to set a string with the current errno message
std::string get_errno_message(const std::string &prefix = "") {
    return prefix + std::strerror(errno);
}

debug::cmd_status debug::_exec_command(const std::string &cmd,
    const std::vector<std::string> &args, const std::string &input)
{
    cmd_status status = {"", "", 1}; // Default to failure

    // Initialize all required pipes
    for (auto & i : pipes)
    {
        if (pipe(i) == -1) {
            status.fd_stderr += get_errno_message("pipe() failed: ");
            status.exit_status = 1;
            return status;
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        // Fork failed
        status.fd_stderr += get_errno_message("fork() failed: ");
        status.exit_status = 1;
        // Close all pipes before returning
        for (auto & pipe : pipes) {
            close(pipe[READ_FD]);
            close(pipe[WRITE_FD]);
        }
        return status;
    }

    if (pid == 0)
    {
        // Child process

        // Redirect stdin
        if (dup2(CHILD_READ_FD, STDIN_FILENO) == -1) {
            perror("dup2 stdin");
            exit(EXIT_FAILURE);
        }

        // Redirect stdout
        if (dup2(CHILD_WRITE_FD, STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            exit(EXIT_FAILURE);
        }

        // Redirect stderr
        if (dup2(CHILD_ERR_FD, STDERR_FILENO) == -1) {
            perror("dup2 stderr");
            exit(EXIT_FAILURE);
        }

        /* Close all pipe fds in the child */
        for (auto & pipe : pipes)
        {
            close(pipe[READ_FD]);
            close(pipe[WRITE_FD]);
        }

        // Build argv for execv
        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(cmd.c_str()));
        for (const auto &arg : args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execv(cmd.c_str(), argv.data());

        // If execv fails
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        // Close unused pipe ends in the parent
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(CHILD_ERR_FD);

        // Set the write end of the stdin pipe to non-blocking to handle potential write errors
        // fcntl(PARENT_WRITE_FD, F_SETFL, O_NONBLOCK); // Optional: Depending on requirements

        // Write to child's stdin
        ssize_t total_written = 0;
        auto input_size = static_cast<ssize_t>(input.size());
        const char *input_cstr = input.c_str();
        ssize_t bytes_to_write = input_size;

        // Ensure input ends with a newline
        std::string modified_input = input;
        if (modified_input.empty() || modified_input.back() != '\n')
        {
            modified_input += "\n";
            input_cstr = modified_input.c_str();
            bytes_to_write = static_cast<ssize_t>(modified_input.size());
        }

        while (total_written < bytes_to_write)
        {
            ssize_t written = write(PARENT_WRITE_FD, input_cstr + total_written, bytes_to_write - total_written);
            if (written == -1)
            {
                if (errno == EINTR)
                    continue; // Retry on interrupt
                else {
                    status.fd_stderr += get_errno_message("write() to child stdin failed: ");
                    status.exit_status = 1;
                    return status;
                }
            }

            total_written += written;
        }

        // Optionally close the write end if no more input is sent
        if (close(PARENT_WRITE_FD) == -1)
        {
            status.fd_stderr += get_errno_message("close() PARENT_WRITE_FD failed: ");
            status.exit_status = 1;
            return status;
        }

        // Function to read all data from a file descriptor
        auto read_all = [&](int fd, std::string &output) -> bool
        {
            char buffer[4096];
            ssize_t count;
            while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
                output.append(buffer, count);
            }

            if (count == -1) {
                output += get_errno_message("read() failed: ");
                return false;
            }
            return true;
        };

        // Read from child's stdout
        if (!read_all(PARENT_READ_FD, status.fd_stdout))
        {
            status.fd_stderr += get_errno_message("read_all() failed: ");
            status.exit_status = 1;
            return status;
        }

        // Read from child's stderr
        if (!read_all(PARENT_ERR_FD, status.fd_stderr))
        {
            status.fd_stderr += get_errno_message("read_all() failed: ");
            status.exit_status = 1;
            return status;
        }

        // Close the read ends
        if (close(PARENT_READ_FD) == -1)
        {
            status.fd_stderr += get_errno_message("close() PARENT_READ_FD failed: ");
            status.exit_status = 1;
            return status;
        }

        if (close(PARENT_ERR_FD) == -1)
        {
            status.fd_stderr += get_errno_message("close() PARENT_ERR_FD failed: ");
            status.exit_status = 1;
            return status;
        }

        // Wait for child process to finish
        int wstatus;
        if (waitpid(pid, &wstatus, 0) == -1)
        {
            status.fd_stderr += get_errno_message("waitpid() failed: ");
            status.exit_status = 1;
            return status;
        }
        else
        {
            if (WIFEXITED(wstatus)) {
                status.exit_status = WEXITSTATUS(wstatus);
            } else if (WIFSIGNALED(wstatus)) {
                std::ostringstream oss;
                oss << "Child terminated by signal " << WTERMSIG(wstatus) << "\n";
                status.fd_stderr += oss.str();
                status.exit_status = 1;
            } else {
                // Other cases like stopped or continued
                status.fd_stderr += "Child process ended abnormally.\n";
                status.exit_status = 1;
            }
        }

        return status;
    }
}

void debug::_log(const __uint128_t& param)
{
    if (param == 0) {
        std::cerr << "0";
        return;
    }

    std::string str;
    __uint128_t tmp = param;

    while (tmp > 0) {
        const char digit = '0' + static_cast<char>(tmp % 10);
        str += digit;
        tmp /= 10;
    }

    std::reverse(str.begin(), str.end());
    std::cerr << str;
}

size_t max_line_length(const std::string& input)
{
    size_t max_length = 0;
    size_t current_length = 0;

    for (char ch : input) {
        if (ch == '\n') {
            if (current_length > max_length) {
                max_length = current_length;
            }
            current_length = 0;
        } else {
            ++current_length;
        }
    }

    if (current_length > max_length) {
        max_length = current_length;
    }

    return max_length;
}

std::string initialize_error_msg(const std::string& msg, const int _errno)
{
    std::string result;
    if (debug::verbose)
    {
        std::ostringstream err_msg;

        const std::string current_time = debug::get_current_date_time();
        err_msg << "Exception Thrown at " << current_time << "\n";

        auto replace_all = [](std::string& input, const std::string& target,
                               const std::string& replacement) {
            if (target.empty())
                return;
            size_t pos = 0;
            while ((pos = input.find(target, pos)) != std::string::npos) {
                input.replace(pos, target.length(), replacement);
                pos += replacement.length();
            }
        };

        std::string processed_msg = msg;
        if (!processed_msg.empty() && processed_msg.back() == '\n') {
            processed_msg.pop_back();
        }
        replace_all(processed_msg, "\n", "\n>>> ");

        err_msg << "Error description:\n>>> " << processed_msg << "\n";
        err_msg << "System Error: errno=" << _errno << ": " << strerror(_errno)
                << "\n";
        const std::regex pattern(R"(([^\(]+)\(([^\)]*)\) \[([^\]]+)\])");
        std::smatch matches;

        err_msg << "\nBacktrace starts here:\n";
        auto frame_backtrace = debug::obtain_stack_frame();

        for (size_t i = 0; i < frame_backtrace.size(); ++i)
        {
            std::stringstream prefix;
            prefix << "Frame #" << i << " " << frame_backtrace[i].second << ": ";
            err_msg << prefix.str();

            if (std::regex_search(frame_backtrace[i].first, matches, pattern)
                && matches.size() > 3)
            {
                const std::string& executable_path = matches[1].str();
                const std::string& traced_address = matches[2].str();
                const std::string& traced_runtime_address
                    = matches[3].str();

                auto generate_addr2line_trace_info
                    = [&](const std::string& address)
                    {
                        auto [fd_stdout, fd_stderr, exit_status]
                            = debug::exec_command("/usr/bin/addr2line", "",
                                "--demangle", "-f", "-p", "-a", "-e",
                                executable_path, address);

                        if (exit_status != 0) {
                            err_msg << "\tObtaining backtrace information failed for "
                                    << executable_path << " with offset "
                                    << address << ": " << fd_stderr
                                    << "\n";
                        } else {
                            std::string caller, path;
                            size_t pos = fd_stdout.find('/');
                            if (pos != std::string::npos) {
                                caller = fd_stdout.substr(0, pos - 4);
                                path = fd_stdout.substr(pos);
                            }

                            size_t pos2 = caller.find('(');
                            if (pos2 != std::string::npos) {
                                caller = caller.substr(0, pos2);
                            }

                            if (!caller.empty() && !path.empty())
                            {
                                const std::string prefix_spaces(prefix.str().length() - 9, ' ');
                                err_msg << caller << "\n" << prefix_spaces << "at " << path;
                            } else {
                                err_msg << fd_stdout;
                            }
                        }
                    };

                if (traced_address.empty()) {
                    generate_addr2line_trace_info(traced_runtime_address);
                } else {
                    generate_addr2line_trace_info(traced_address);
                }
            } else {
                err_msg << "No trace information\n";
            }
            err_msg << "\n";
        }
        err_msg << "Backtrace ends here.\n";

        err_msg << "\nThread Information:\n" << debug::get_verbose_info() << "\n";

        return err_msg.str();
    }

    return ">>> " + msg + " (errno=" + std::to_string(_errno) + ") <<<";
}

SysdarftBaseError::SysdarftBaseError(
    const std::string& msg)
#ifdef __DEBUG__
    : runtime_error(initialize_error_msg(msg, errno))
#else
    : runtime_error(msg + "errno: " + std::to_string(errno))
#endif
    , cur_errno(errno)
{
}

bool isDigits(const std::string& str)
{
    return std::ranges::all_of(str, ::isdigit);
}

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

    size_t pos1 = line.find('(');
    size_t pos2 = line.rfind(')');
    if (pos1 == std::string::npos || pos2 == std::string::npos
        || pos2 <= pos1 + 1) {
        std::cerr << "Malformed stat file for TID " << tid << std::endl;
        return false;
    }

    std::string before = line.substr(0, pos1 - 1);
    std::string comm = line.substr(pos1 + 1, pos2 - pos1 - 1);
    std::string after = line.substr(pos2 + 2);

    std::istringstream iss(after);
    std::vector<std::string> fields;
    std::string field;
    while (iss >> field) {
        fields.push_back(field);
    }

    if (fields.size() < 44) {
        std::cerr << "Not enough fields in stat file for TID " << tid
                  << std::endl;
        return false;
    }

    info.tid = tid;
    info.name = comm;
    info.state = fields[0][0];
    try {
        info.utime = std::stoul(fields[11]);
        info.stime = std::stoul(fields[12]);
        info.starttime = std::stoul(fields[19]);
    } catch (const std::invalid_argument&) {
        std::cerr << "Invalid number in stat file for TID " << tid << std::endl;
        return false;
    } catch (const std::out_of_range&) {
        std::cerr << "Number out of range in stat file for TID " << tid
                  << std::endl;
        return false;
    }

    return true;
}

bool parseStatusFile(pid_t tid, ThreadInfo& info)
{
    std::string statusPath
        = "/proc/self/task/" + std::to_string(tid) + "/status";
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        std::cerr << "Failed to open " << statusPath << std::endl;
        return false;
    }

    std::string line;
    std::map<std::string, std::string> statusMap;
    while (getline(statusFile, line))
    {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            size_t valueStart = line.find_first_not_of(" \t", colon + 1);
            std::string value = (valueStart != std::string::npos)
                ? line.substr(valueStart)
                : "";
            statusMap[key] = value;
        }
    }
    statusFile.close();

    if (statusMap.contains("VmSize")) {
        std::istringstream iss(statusMap["VmSize"]);
        iss >> info.vmsize;
    } else {
        info.vmsize = 0;
    }

    if (statusMap.contains("VmRSS")) {
        std::istringstream iss(statusMap["VmRSS"]);
        iss >> info.vmrss;
    } else {
        info.vmrss = 0;
    }

    return true;
}

bool getThreadInfo(const pid_t tid, ThreadInfo& info)
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

    while ((entry = readdir(dir)) != nullptr)
    {
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

    size_t max_tid = std::string("TID").size();
    size_t max_state = std::string("State").size();
    size_t max_name = std::string("Name").size();
    size_t max_utime = std::string("Utime").size();
    size_t max_stime = std::string("Stime").size();
    size_t max_vmsize = std::string("Vmsize(kB)").size();
    size_t max_vmrss = std::string("Vmrss(kB)").size();
    size_t max_starttime = std::string("Starttime").size();

    for (const auto& t : threads)
    {
        max_tid = std::max(max_tid, std::to_string(t.tid).size());
        max_state = std::max(max_state, std::string(1, t.state).size());
        max_name = std::max(max_name, t.name.size());
        max_utime = std::max(max_utime, std::to_string(t.utime).size());
        max_stime = std::max(max_stime, std::to_string(t.stime).size());
        max_vmsize = std::max(max_vmsize, std::to_string(t.vmsize).size());
        max_vmrss = std::max(max_vmrss, std::to_string(t.vmrss).size());
        max_starttime
            = std::max(max_starttime, std::to_string(t.starttime).size());
    }

    size_t pad_tid = max_tid + 3;
    size_t pad_state = max_state + 3;
    size_t pad_name = max_name + 3;
    size_t pad_utime = max_utime + 3;
    size_t pad_stime = max_stime + 3;
    size_t pad_vmsize = max_vmsize + 3;
    size_t pad_vmrss = max_vmrss + 3;
    size_t pad_starttime = max_starttime + 3;

    auto create_spaces = [](const size_t count) -> std::string {
        return std::string(count, ' ');
    };

    auto pad_right
        = [&](const std::string& s, const size_t width) -> std::string {
        if (s.size() >= width) {
            return s;
        }
        return s + create_spaces(width - s.size());
    };

    ret << "Number of threads in current process: " << threads.size() << "\n\n";

    ret << "Detailed Thread Information:\n";
    ret << "-----------------------------------------------\n";

    std::stringstream table;
    table << pad_right("TID", pad_tid) << pad_right("State", pad_state)
          << pad_right("Name", pad_name) << pad_right("Utime", pad_utime)
          << pad_right("Stime", pad_stime)
          << pad_right("Vmsize(kB)", pad_vmsize)
          << pad_right("Vmrss(kB)", pad_vmrss)
          << pad_right("Starttime", pad_starttime) << "\n";
    ret << table.str();

    ret << std::string(table.str().length() - 1, '-') << "\n";

    for (const auto& thread : threads) {
        ret << pad_right(std::to_string(thread.tid), pad_tid)
            << pad_right(std::string(1, thread.state), pad_state)
            << pad_right(thread.name.substr(0, 15), pad_name)
            << pad_right(std::to_string(thread.utime), pad_utime)
            << pad_right(std::to_string(thread.stime), pad_stime)
            << pad_right(std::to_string(thread.vmsize), pad_vmsize)
            << pad_right(std::to_string(thread.vmrss), pad_vmrss)
            << pad_right(std::to_string(thread.starttime), pad_starttime)
            << "\n";
    }

    return ret.str();
}
