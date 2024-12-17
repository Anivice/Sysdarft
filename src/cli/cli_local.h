#ifndef CLI_LOCAL_H
#define CLI_LOCAL_H

#include <string>
#include <vector>
#include <module.h>
#include <global.h>

// A structure for argument behavior
struct ArgumentInfo {
    bool use_file_completion;                  // If true, use file completion
    std::vector<std::string> static_completions; // A fixed set of completions for this argument
    bool dynamic_completions;                  // If true, generate completions dynamically at runtime
};

// A structure for command behavior
struct CommandInfo {
    std::string name;
    // Each argument position can have different behavior
    std::vector<ArgumentInfo> arguments;
};

const std::vector<CommandInfo> commands = {
    {
        "help", { }
    },
    {
        "exit", { }
    },
    {
        "load_module",
        {
                {true, {}, false},
                {true, {}, false},
            }
    },
    {
        "unload_module",
        {
                {false, {}, true},
                {false, {}, true},
            }
    },
    {
        "list_modules", { }
    },
    {
        "load_config", {
                {true, {}, false},
                {true, {}, false},
            }
    },
    {
        "show_config", { }
    },
    {
        "test_curses", { }
    },
    {
        "ls", { }
    },
};

std::vector<std::string> splitAndDiscardEmpty(const std::string& str);
char** custom_completion(const char* text, int start, int end);

extern std::unordered_map < std::string, Module > loaded_modules;

#endif //CLI_LOCAL_H
