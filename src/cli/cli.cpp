#include <module.h>
#include <cli.h>
#include <chrono>
#include <complex>
#include <iostream>
#include <thread>
#include <readline/readline.h>
#include <readline/history.h>
#include <debug.h>
#include <cmath>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <filesystem>

#define INPUT_INSTANCE_NAME "InputProcessor"
#define INPUT_METHOD_NAME "Input"

std::map < std::string, Module > loaded_modules;


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

std::vector<CommandInfo> commands = {
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
            {false, {}, true}
        }
    },
    {
        "list_modules", { }
    }
};

std::vector<std::string> get_command_names()
{
    std::vector<std::string> names;
    for (const auto& c : commands) {
        names.push_back(c.name);
    }
    return names;
}

const CommandInfo* find_command_info(const std::string& name)
{
    for (const auto& command : commands) {
        if (command.name == name) return &command;
    }
    return nullptr;
}

// A helper function to produce dynamic completions based on command context
// Here we customize results depending on previously entered arguments.
std::vector<std::string> get_dynamic_completions(const std::string& cmd_name,
                                                 const std::vector<std::string>& words,
                                                 int arg_index)
{
    std::vector<std::string> results;

    if (cmd_name == "unload_module") {
        for (const auto& module : loaded_modules) {
            results.emplace_back(module.first);
        }
    }

    return results;
}

// We'll store context for dynamic completions so that the completer can access it.
static std::string current_cmd;
static std::vector<std::string> current_words;
static int current_arg_index;

char* dynamic_arg_completer(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;

        std::string prefix(text);
        // Use the command and words to get command-specific dynamic completions
        auto dynamic = get_dynamic_completions(current_cmd, current_words, current_arg_index);

        // Filter completions by prefix
        for (const auto& candidate : dynamic) {
            if (candidate.rfind(prefix, 0) == 0) {
                matches.push_back(candidate);
            }
        }
    }

    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

char* static_arg_completer(const char* text, int state, const std::vector<std::string>& completions)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;
        std::string prefix(text);
        for (const auto& c : completions) {
            if (c.rfind(prefix, 0) == 0) {
                matches.push_back(c);
            }
        }
    }

    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

char* command_name_completer(const char* text, int state)
{
    static std::vector<std::string> cmd_matches;
    static size_t match_index = 0;

    if (state == 0) {
        cmd_matches.clear();
        match_index = 0;
        std::string prefix(text);
        for (auto& cmd : get_command_names()) {
            if (cmd.rfind(prefix, 0) == 0) {
                cmd_matches.push_back(cmd);
            }
        }
    }

    if (match_index < cmd_matches.size()) {
        return strdup(cmd_matches[match_index++].c_str());
    } else {
        return nullptr;
    }
}

static const std::vector<std::string>* current_completions = nullptr;
static const char * empty_string = "";

char* static_completions_proxy(const char* t, int s)
{
    if (current_completions) {
        return static_arg_completer(t, s, *current_completions);
    } else {
        return const_cast<char *>(empty_string);
    }
}

// Main completion function
char** custom_completion(const char* text, int start, int end)
{
    // Parse the line
    char* buffer = strndup(rl_line_buffer, end);
    std::string input(buffer);
    free(buffer);

    std::istringstream iss(input);
    std::vector<std::string> words{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

    // If we're completing the first word (command)
    if (words.empty() || (words.size() == 1 && start == 0)) {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, command_name_completer);
    }

    // Identify the command and argument
    const CommandInfo* cmd_info = find_command_info(words[0]);
    if (!cmd_info) {
        // Unknown command
        rl_attempted_completion_over = 1;
        return nullptr;
    }

    int arg_index = static_cast<int>(words.size()) - 1;
    if (arg_index < 0 || arg_index >= (int)cmd_info->arguments.size()) {
        // No argument info defined at this position
        rl_attempted_completion_over = 1;
        return nullptr;
    }

    const ArgumentInfo& arg_info = cmd_info->arguments[arg_index];

    // Store context for dynamic completion
    current_cmd = words[0];
    current_words = words;
    current_arg_index = arg_index;

    // File completion
    if (arg_info.use_file_completion) {
        rl_attempted_completion_over = 0; // Allow default filename completion
        return nullptr;
    }

    // Static completion
    if (!arg_info.static_completions.empty()) {
        rl_attempted_completion_over = 1;
        current_completions = &arg_info.static_completions;
        return rl_completion_matches(text, static_completions_proxy);
    }

    // Dynamic completion
    if (arg_info.dynamic_completions) {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, dynamic_arg_completer);
    }

    rl_attempted_completion_over = 1;
    return nullptr;
}

std::vector<std::string> splitAndDiscardEmpty(const std::string& str)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (getline(iss, token, ' ')) {
        if (!token.empty()) {  // This check ensures no empty strings are added
            tokens.push_back(token);
        }
    }

    return tokens;
}

void Cli::run()
{
    rl_attempted_completion_function = custom_completion;

    char* input;
    while ((input = readline("sysdarft> ")) != nullptr)
    {
        if (*input)
        {
            std::lock_guard lock(access_mutex);
            GlobalEventProcessor(INPUT_INSTANCE_NAME,
                INPUT_METHOD_NAME)(splitAndDiscardEmpty(input));

            if (last_command != input) {
                last_command = input;
                add_history(input);
            }
        }

        free(input);
    }

    debug::log("Console input turned off, exiting...\n");
    std::lock_guard lock(access_mutex);
    GlobalEventProcessor("Global", "destroy")();
}

class cleanup_handler_ {
public:
    void destroy()
    {
        debug::log("Cleaning up...\n");
        debug::log("Stage 1: unloading modules...\n");
        for (auto& module : loaded_modules) {
            debug::log("Unloading module ", module.first, "...\n");
            module.second.unload();
            debug::log("...done\n");
        }

        debug::log("All stage completed!\n");
        exit(EXIT_SUCCESS);
    }
} cleanup_handler;

class input_processor_ {
public:
    void process_input(const std::vector < std::string > & args)
    {
        if (args.empty()) {
            return;
        }

        if (args.at(0) == "help")
        {
            debug::log(
               "help                        print this help menu\n"
               "exit                        exit program\n"
               "load_module [Module Path]   load module\n"
               "unload_module [Module]      load module\n"
               "list_modules [Module]       list modules\n");
        }
        else if (args.at(0) == "exit")
        {
            debug::log("Console requested termination. Exiting...\n");
            exit(EXIT_SUCCESS);
        }
        else if (args.at(0) == "load_module")
        {
            if (args.size() != 2) {
                debug::log("load_module [Module Path]\n");
                return;
            }

            auto path_to_name = [](const std::string & module_name)->std::string
            {
                std::regex pattern(R"(.*lib([A-Za-z0-9_]+)\.so$)");
                std::smatch matches;
                if (std::regex_match(module_name, matches, pattern)) {
                    if (matches.empty()) {
                        return "";
                    }

                    return matches[1];
                }

                return "";
            };

            const std::string module_path = args.at(1);
            const std::string module_name = path_to_name(args.at(1));

            if (loaded_modules.contains(module_name)) {
                debug::log("Module already loaded\n");
                return;
            }

            debug::log("Loading module: ", module_path, "...\n");
            try {
                Module module(module_path);
                loaded_modules.emplace(module_name, module);
                debug::log("Module '", module_name, "' loaded.\n");
            } catch (const SysdarftBaseError & err) {
                debug::log("Error encountered during loading module: ", err.what(), "\n");
            }
        }
        else if (args.at(0) == "unload_module")
        {
            if (args.size() != 2) {
                debug::log("unload_module [Module]\n");
                return;
            }

            if (!loaded_modules.contains(args.at(1))) {
                debug::log("Module not loaded\n");
                return;
            }

            debug::log("Unloading module: ", args.at(1), "\n");
            try {
                Module module = loaded_modules.at(args.at(1));
                module.unload();
                loaded_modules.erase(args.at(1));
            } catch (const SysdarftBaseError & err) {
                debug::log("Error encountered during loading module: ", err.what(), "\n");
            }
        }
        else if (args.at(0) == "list_modules")
        {
            debug::log("A total of ", loaded_modules.size(), " module(s) loaded.\n");
            for (const auto& module : loaded_modules) {
                debug::log(module.first, "\n");
            }
        }
        else
        {
            debug::log("Unknown command: ", args.at(0), "\n");
            debug::log("Use `help` to show the usage.\n");
        }
    }
} input_processor;

Cli::Cli()
{
    GlobalEventProcessor.install_instance(INPUT_INSTANCE_NAME, &input_processor,
        INPUT_METHOD_NAME, &input_processor_::process_input);
    GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &cleanup_handler,
        GLOBAL_DESTROY_METHOD_NAME, &cleanup_handler_::destroy);

    std::thread CliWorkThread(&Cli::run, this);
    CliWorkThread.detach();
}
