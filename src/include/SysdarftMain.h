#ifndef SYSDARFTMAIN_H
#define SYSDARFTMAIN_H
#undef log
#include <SysdarftDebug.h>
#include <SysdarftModule.h>
#include <EncodingDecoding.h>
#include <SysdarftCPU.h>
#undef OK // defined by ncurses, conflict with crow
#undef log

#include <getopt.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <cstring>
#include <memory>
#include <csignal>
#include <crow.h>
// remove debug logging.
// Manual activation can be done by including SysdarftDebug.h again after this header
// should further requirement of log() needed.
// Won't override any functions, just marco since, preprocessor won't be able to do that

struct option_complicated
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
    const char * arg_explain;
};

class SysdarftDisassemblerError final : public SysdarftBaseError
{
public:
    explicit SysdarftDisassemblerError(const std::string & message) :
        SysdarftBaseError("Disassembler failed to process data: " + message) { }
};

const option_complicated long_options[] = {
    {"help",    no_argument,        nullptr, 'h',   "Show this help message"},
    {"version", no_argument,        nullptr, 'v',   "Output version information"},
    {"module",  required_argument,  nullptr, 'm',   "Load a module\n"
                                                                                                "This option can be used multiple times\n"
                                                                                                "to load multiple modules"},
    {"verbose", no_argument,        nullptr, 'V',   "Enable verbose mode" },
    {"compile", required_argument,  nullptr, 'c',   "Compile a file\n"
                                                                                                "This option can be used multiple times\n"
                                                                                                "to compile multiple files into one single binary"},
    {"output",  required_argument,  nullptr, 'o',   "Compilation output file"},
    {"format",  required_argument,  nullptr, 'f',   "Compile format. It can be bin, exe, or sys"},
    {"disassem",required_argument,  nullptr, 'd',   "Disassemble a file"},
    {"origin",  required_argument,  nullptr, 'g',   "Redefine origin for disassembler\n"
                                                                                                "When left unset, origin is 0"},
    {"bios",    required_argument,  nullptr, 'b',   "Specify a BIOS firmware binary"},
    {"hdd",     required_argument,  nullptr, 'L',   "Specify a Hard Disk"},
    {"fda",     required_argument,  nullptr, 'A',   "Specify floppy disk A"},
    {"fdb",     required_argument,  nullptr, 'B',   "Specify floppy disk B"},
    {"memory",  required_argument,  nullptr, 'M',   "Specify memory size (in MB)\n"
                                                                                                "Left unset and default size is 32MB"},
    {"boot",    no_argument,        nullptr, 'S',   "Boot the system"},
    {"debug",   required_argument,  nullptr, 'D',   "Boot the system with remote debug console\n"
                                                                                                "The system will not be started unless the debug console is connected\n"
                                                                                                "Disconnecting debug console will cause system to immediately halt,\n"
                                                                                                "which is equivalent to pulling the plug\n"
                                                                                                "Debug server expects this argument: [Debug Server IP Address]:[Port]"},
    {"crow-log",required_argument,  nullptr, 0,     "Specify the log file of crow service"},
    {nullptr,   0,                  nullptr, 0,     nullptr }
};

using ParsedOptions = std::map<std::string, std::vector<std::string>>;
using ParsedArgs = std::pair<ParsedOptions, std::vector<std::string>>;
ParsedArgs get_args(int argc, char** argv, option long_options[]);
void compile_to_binary(const std::vector< std::string > &, const std::string &);
void disassemble(const std::string &, uint64_t);
std::string show_context(SysdarftCPU &, uint8_t, const SysdarftCPU::WidthAndOperandsType &);

#define CONTINUE        (0x00)
#define SHOW_CONTEXT    (0x01)

#define do_action(action, ...) std::vector < uint8_t > { static_cast<uint8_t>(action), __VA_ARGS__ }

class RemoteDebugServer {
private:
    struct SSEConnection {
        std::shared_ptr<crow::response> resPtr;
        bool isOpen;
    };

    SysdarftCPU & CPUInstance;
    crow::SimpleApp JSONBackend;
    std::thread server_thread;
    std::map < std::pair < uint64_t /* CB */, uint64_t /* IP */ > /* breakpoint */,
        std::vector < uint8_t > /* conditional byte code */ > breakpoint_list;
    std::atomic < bool > skip_bios_ip_check = false;
    std::ofstream crow_log;
    std::mutex sseMutex;
    std::vector<SSEConnection> sseClients;
    std::mutex action_access_mutex;
    std::vector < uint8_t > debug_action_request;
    std::mutex action_result_access_mutex;
    std::string debug_action_result;

    std::vector < uint8_t > compile_conditional_expression_to_byte_code(const std::string & conditional_expression);
    bool is_condition_met(std::vector < uint8_t > condition_expression_byte_code);

    template < typename... Args >
    std::string invoke_action(const uint8_t action, const Args &... args)
    {
        {
            const auto action_code = do_action(action, args...);
            std::lock_guard<std::mutex> lock(action_access_mutex);
            debug_action_request = action_code;
        }

        std::string result;

        while (result.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(action_result_access_mutex);
            result = debug_action_result;
        }

        {
            std::lock_guard<std::mutex> lock(action_result_access_mutex);
            debug_action_result.clear();
        }

        return result;
    }

    class SysdarftLogHandler final : public crow::ILogHandler
    {
    private:
        std::ostream & custom_stream;
        const bool log_available;

    public:
        explicit SysdarftLogHandler(std::ostream & custom_stream_, const bool log_available_)
            : custom_stream(custom_stream_), log_available(log_available_) { }

        void log(std::string message, crow::LogLevel level) override
        {
            if (!log_available) {
                return;
            }

            const char* level_text = nullptr;
            switch (level) {
            case crow::LogLevel::Debug:     level_text = "[DEBUG]";   break;
            case crow::LogLevel::Info:      level_text = "[INFO]";    break;
            case crow::LogLevel::Warning:   level_text = "[WARN]";    break;
            case crow::LogLevel::Error:     level_text = "[ERROR]";   break;
            case crow::LogLevel::Critical:  level_text = "[CRITICAL]";break;
            default:                        level_text = "[LOG]";     break;
            }

            custom_stream << level_text << " " << message << std::endl;
        }
    } SysdarftLogHandlerInstance;

public:
    RemoteDebugServer(const std::string & ip,
        uint16_t port,
        SysdarftCPU & _CPUInstance,
        const std::string & crow_log_file);
    ~RemoteDebugServer();
    RemoteDebugServer(const RemoteDebugServer &) = delete;
    RemoteDebugServer(RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(const RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(const RemoteDebugServer &) = delete;

    bool if_breakpoint(__uint128_t);
    void at_breakpoint(__uint128_t, uint8_t, const SysdarftCPU::WidthAndOperandsType &);
};

#endif //SYSDARFTMAIN_H
