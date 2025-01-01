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
#include <SysdarftDebug.h>

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

debug::cmd_status debug::_exec_command(
    const std::string& cmd, const std::vector<std::string>& args)
{
    debug::cmd_status error_status
        = { .fd_stdout = "", .fd_stderr = "", .exit_status = 1 };

    int pipe_stdout[2];
    int pipe_stderr[2];
    if (pipe(pipe_stdout) != 0 || pipe(pipe_stderr) != 0) {
        error_status.fd_stderr = "Failed to create pipe for stdout!";
    }

    const pid_t pid = fork();
    if (pid == -1) {
        error_status.fd_stderr = "Failed to fork process!";
    }

    if (pid == 0) { // Child process
        dup2(pipe_stdout[1], STDOUT_FILENO);
        close(pipe_stdout[0]);
        close(pipe_stdout[1]);

        dup2(pipe_stderr[1], STDERR_FILENO);
        close(pipe_stderr[0]);
        close(pipe_stderr[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(cmd.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(cmd.c_str(), argv.data());
        exit(EXIT_FAILURE);
    }

    close(pipe_stdout[1]);
    close(pipe_stderr[1]);

    cmd_status result;
    char buffer[256];
    ssize_t count;

    while ((count = read(pipe_stdout[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        result.fd_stdout += buffer;
    }
    close(pipe_stdout[0]);

    while ((count = read(pipe_stderr[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        result.fd_stderr += buffer;
    }
    close(pipe_stderr[0]);

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        result.exit_status = WEXITSTATUS(status);
    } else {
        result.exit_status = -1;
    }

    return result;
}

void debug::_log(const __uint128_t& param)
{
    if (param == 0) {
        std::cout << "0";
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
    std::cout << str;
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
                            = debug::exec_command("addr2line",
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

std::string last_sysdarft_error_;

SysdarftBaseError::SysdarftBaseError(
    const std::string& msg)
    : runtime_error(initialize_error_msg(msg, errno))
    , cur_errno(errno)
{
    last_sysdarft_error_ = this->runtime_error::what();
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

void handle_sigabrt(int signum)
{
    const char* prefix
        = "[FATAL ERROR] Program is terminated using SIGABRT (Signal Abort)!\n";
#ifdef __DEBUG__
    debug::verbose = true;
    const SysdarftBaseError Error(
        "Abnormal termination!\nLast captured error:\n" + last_sysdarft_error_
        + "\n");
    const std::string str = Error.what();
    write(STDERR_FILENO, prefix, strlen(prefix));
    write(STDERR_FILENO, str.c_str(), str.length() - 1);
#endif // __DEBUG__
    _exit(EXIT_FAILURE);
}

class SysdarftDebugInitialization {
public:
    SysdarftDebugInitialization()
    {
        struct sigaction sa { };
        sa.sa_handler = handle_sigabrt;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(SIGABRT, &sa, nullptr) == -1) {
            perror("Error setting up SIGABRT handler");
            exit(EXIT_FAILURE);
        }
    }
} SysdarftDebugInitializationInstance;
