/* SysdarftMain.h
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
#include <utility>
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
    {"regex",   no_argument,        nullptr, 'R',   "If .equ preprocessor will be using regular expression\n"
                                                                                                "If this option is not set, .equ will simply replace the string\n"
                                                                                                "If this option is set, .equ will be processed\n"
                                                                                                "using `/usr/bin/sed -E 's/../../g'`\n"
                                                                                                "This will, of course, introduce performance downgrade when compiling"},
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
void compile_to_binary(const std::vector< std::string > &, const std::string &, bool);
void disassemble(const std::string &, uint64_t);
std::string show_context(SysdarftCPU &, uint64_t actual_ip, uint8_t, const SysdarftCPU::WidthAndOperandsType &);

class RemoteDebugServer {
private:
    struct ConditionalTargetExpression {
        std::string TgExp1;
        std::string TgExp2;
        std::vector < uint8_t > EncodedTgExp1;
        std::vector < uint8_t > EncodedTgExp2;
        std::unique_ptr<ConditionalTargetExpression> ComplexTgExp1;
        std::unique_ptr<ConditionalTargetExpression> ComplexTgExp2;
        int ConditionType;  // Codes for different conditions
    };

    enum ConditionTypes {
        VAL_EQUAL,
        AND,
        OR,
        NOT,
        NONE
    };

    class Parser {
    public:
        explicit Parser(std::string input) : input(std::move(input)), pos(0) {}
        std::unique_ptr<ConditionalTargetExpression> parseExpression();

    private:
        std::string input;
        size_t pos;

        void skipWhitespace();
        bool match(const std::string& keyword);
        std::unique_ptr<ConditionalTargetExpression> parseValEqual();
        std::unique_ptr<ConditionalTargetExpression> parseLogical(int);

        // Regex processing remains unchanged
        std::string processTgExp(const std::string& exp);
        std::string parseUntilCommaOrParen();
        std::string parseUntilParen();
    };

    struct SSEConnection {
        std::shared_ptr<crow::response> resPtr;
        bool isOpen;
    };

    SysdarftCPU & CPUInstance;
    crow::SimpleApp JSONBackend;
    std::thread server_thread;

    std::mutex g_br_list_access_mutex;
    std::map < std::pair < uint64_t /* CB */, uint64_t /* IP */ > /* breakpoint */,
        std::string /* condition */ > breakpoint_list;
    std::vector < std::pair < std::vector < uint8_t > /* target */, uint64_t /* last value */ > > watch_list;

    std::atomic < bool > skip_bios_ip_check = false;
    std::atomic < bool > breakpoint_triggered = false;
    std::atomic < bool > manual_stop = false;
    std::atomic < bool > stepi = false;
    std::ofstream crow_log;
    std::mutex sseMutex;
    std::vector<SSEConnection> sseClients;
    std::atomic < uint64_t > actual_ip;
    std::atomic < uint8_t > opcode;
    std::atomic < const SysdarftCPU::WidthAndOperandsType * > args;

    bool is_condition_met(const std::string &);
    uint64_t absolute_target_access(const std::vector < uint8_t > &) const;
    void absolute_target_store(const std::vector < uint8_t > &, uint64_t);
    bool check_one_level_of_condition(const ConditionalTargetExpression * expr);

    class SysdarftLogHandler final : public crow::ILogHandler
    {
    private:
        std::ostream & custom_stream;
        const bool log_available;

    public:
        explicit SysdarftLogHandler(std::ostream & custom_stream_, const bool log_available_)
            : custom_stream(custom_stream_), log_available(log_available_) { }
        void log(std::string message, crow::LogLevel level) override;
    } SysdarftLogHandlerInstance;

    void crow_setup_action();
    void crow_setup_continue();
    void crow_setup_showContext();
    void crow_setup_intAlertSSE();
    void crow_setup_isAPIAvailable();
    void crow_setup_setBreakpoint();
    void crow_setup_showBreakpoint();
    void crow_setup_stepi();
    void crow_setup_watcher();
    void crow_setup_disassemble_an_area();
    void crow_setup_kbint();

public:
    RemoteDebugServer(const std::string &,
        uint16_t,
        SysdarftCPU &,
        const std::string &);
    ~RemoteDebugServer();
    RemoteDebugServer(const RemoteDebugServer &) = delete;
    RemoteDebugServer(RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(const RemoteDebugServer &&) = delete;
    RemoteDebugServer & operator=(const RemoteDebugServer &) = delete;

    bool if_breakpoint(__uint128_t);
    void at_breakpoint(__uint128_t, uint64_t, uint8_t, const SysdarftCPU::WidthAndOperandsType &);
};

inline void remove_spaces(std::string &input)
{
    input.erase(std::ranges::remove_if(input, ::isspace).begin(), input.end());
}

std::string disassemble_code(std::vector < uint8_t > assembled_code, uint64_t org);

#endif //SYSDARFTMAIN_H
