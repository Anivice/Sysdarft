#ifndef SYSDARFTMAIN_H
#define SYSDARFTMAIN_H

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
#include <SysdarftDebug.h>
#include <SysdarftModule.h>
#include <EncodingDecoding.h>
#include <SysdarftCPU.h>

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
    {"help",    no_argument,       nullptr, 'h',    "Show this help message"},
    {"version", no_argument,       nullptr, 'v',    "Output version information"},
    {"module",  required_argument, nullptr, 'm',    "Load a module\n"
                                                                                                "This option can be used multiple times\n"
                                                                                                "to load multiple modules"},
    {"verbose", no_argument,       nullptr, 'V',    "Enable verbose mode" },
    {"compile", required_argument, nullptr, 'c',    "Compile a file\n"
                                                                                                "This option can be used multiple times\n"
                                                                                                "to compile multiple files into one single binary"},
    {"output",  required_argument,  nullptr, 'o',   "Output file for compilation"},
    {"format",  required_argument,  nullptr, 'f',   "Compile format. It can be bin, exe, or sys"},
    {"disassem",required_argument,  nullptr, 'd',   "Disassemble a file"},
    {"origin",  required_argument,  nullptr, 'g',   "Redefine origin for disassembler\n"
                                                                                                "When left unset, origin is 0."},
    {"bios",    required_argument,  nullptr, 'b',   "Specify a BIOS firmware binary"},
    {"hdd",     required_argument,  nullptr, 'L',   "Specify a Hard Disk"},
    {"fda",     required_argument,  nullptr, 'A',   "Specify floppy disk A"},
    {"fdb",     required_argument,  nullptr, 'B',   "Specify floppy disk B"},
    {"memory",  required_argument,  nullptr, 'M',   "Specify memory size\n"
                                                                                              "Left unset and default size is 32MB"},
    {"boot",    no_argument,        nullptr, 'S',   "Boot the system"},
    {nullptr,   0,                  nullptr,  0,    nullptr }
};

using ParsedOptions = std::map<std::string, std::vector<std::string>>;
using ParsedArgs = std::pair<ParsedOptions, std::vector<std::string>>;
ParsedArgs get_args(int argc, char** argv, option long_options[]);
void compile_to_binary(const std::vector< std::string > &, const std::string &);
void disassemble(const std::string &, uint64_t);

#endif //SYSDARFTMAIN_H
