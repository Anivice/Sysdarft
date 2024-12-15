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

#define INPUT_INSTANCE_NAME "InputProcessor"
#define INPUT_METHOD_NAME "Input"

std::vector<std::string> commands = {"help", "exit", "load_module", "unload_module", "list_modules"};

bool is_command(const std::string& input)
{
    for (const auto& cmd : commands) {
        if (input == cmd) return true;
    }
    return false;
}

// Custom completer for commands
char* command_completer(const char* text, int state)
{
    static std::vector<std::string> matches;
    static size_t match_index = 0;

    if (state == 0) {
        matches.clear();
        match_index = 0;

        const std::string text_str(text);
        for (const auto& cmd : commands)
        {
            if (std::string(cmd).find(text_str) == 0) {
                matches.push_back(cmd);
            }
        }
    }

    if (match_index >= matches.size()) {
        return nullptr;
    } else {
        return strdup(matches[match_index++].c_str());
    }
}

// Custom readline completion function
char** custom_completion(const char* text, int start, int end)
{
    char **matches = nullptr;

    if (start == 0) {
        matches = rl_completion_matches(text, command_completer);
    } else {
        rl_attempted_completion_over = 0; // Let Readline handle file completion
    }

    return matches;
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

std::map < std::string, Module > loaded_modules;

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
               "unload_module [Module]      load module\n");
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

            if (loaded_modules.contains(args.at(1))) {
                debug::log("Module already loaded\n");
                return;
            }

            debug::log("Loading module: ", args.at(1), "\n");
            try {
                Module module(args.at(1));
                loaded_modules.emplace(args.at(1), module);
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
            debug::log("A total of ", loaded_modules.size(), " modules loaded.\n");
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

    std::thread CliWorkThread(&Cli::run, this);
    CliWorkThread.detach();
}
