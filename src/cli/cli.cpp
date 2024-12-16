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
#include <csignal>
#include <atomic>
#include <fstream>

std::unordered_map< std::string, Module > loaded_modules;
std::atomic<bool> gSigintReceived(false);
std::string prefix = "sysdarft> ";
std::string history_file;
std::vector<std::string> history_lines;

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

    const auto&[use_file_completion, static_completions, dynamic_completions] =
        cmd_info->arguments[arg_index];

    // Store context for dynamic completion
    current_cmd = words[0];
    current_words = words;
    current_arg_index = arg_index;

    // File completion
    if (use_file_completion) {
        rl_attempted_completion_over = 0; // Allow default filename completion
        return nullptr;
    }

    // Static completion
    if (!static_completions.empty()) {
        rl_attempted_completion_over = 1;
        current_completions = &static_completions;
        return rl_completion_matches(text, static_completions_proxy);
    }

    // Dynamic completion
    if (dynamic_completions) {
        rl_attempted_completion_over = 1;
        return rl_completion_matches(text, dynamic_arg_completer);
    }

    rl_attempted_completion_over = 1;
    return nullptr;
}

std::vector<std::string> splitAndDiscardEmpty(const std::string& str) {
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

// Signal handler function
void handle_sigint(int signum)
{
    if (signum == SIGINT)
    {
        gSigintReceived.store(true);
        // Optionally, write a message directly to STDOUT to avoid using non-signal-safe functions
        const std::string local_prefix = "\n" + prefix;
        write(STDOUT_FILENO, local_prefix.c_str(), local_prefix.size());

        // Inform Readline to abort the current input
        rl_done = 1;
    }
}

void Cli::run()
{
    // Setup sigaction for SIGINT
    struct sigaction sa{};
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // You can set SA_RESTART if desired

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        throw SysdarftBaseError("Installing signal handler failed!");
    }

    rl_attempted_completion_function = custom_completion;
    rl_catch_signals = 0;

    if (debug::verbose) {
        prefix = _RED_ _BOLD_ _FLASH_ "(VERBOSE) " _REGULAR_ "sysdarft> ";
    }

    const char* home = std::getenv("HOME");

    if (home == nullptr) {
        debug::log("HOME not set!\n");

    } else {
        history_file = home;
        history_file += "/.sysdarft";
    }

    std::ifstream hfile(history_file, std::ios::in);
    while (hfile.is_open() && hfile.eof() == false) {
        std::string line;
        std::getline(hfile, line);
        if (!line.empty()) {
            add_history(line.c_str());
        }
    }

    if (hfile.is_open()) {
        hfile.close();
    }

    char* input;
    while ((input = readline(prefix.c_str())) != nullptr)
    {
        if (gSigintReceived.load()) {
            gSigintReceived.store(false);
            free(input);
            continue;
        }

        if (*input)
        {
            std::lock_guard lock(access_mutex);
            auto cleaned_vec = splitAndDiscardEmpty(input);
            GlobalEventProcessor(GLOBAL_INSTANCE_NAME,
                GLOBAL_INPUT_METHOD_NAME)(cleaned_vec);

            std::string current;
            for (const auto& word : cleaned_vec) {
                current += word + " ";
            }

            if (!current.empty()) {
                current.pop_back();
            }

            if (current != last_command) {
                last_command = current;
                history_lines.emplace_back(current);
                add_history(current.c_str());
            }
        }

        free(input);
    }

    debug::log("Console input turned off, exiting...\n");
    std::lock_guard lock(access_mutex);
    GlobalEventProcessor(GLOBAL_INSTANCE_NAME, GLOBAL_DESTROY_METHOD_NAME)();
}

class cleanup_handler_ {
public:
    void destroy()
    {
        debug::log("Cleaning up...\n");
        debug::log("Stage 1: unloading modules...\n");
        int index = 1;
        for (auto&[fst, snd] : loaded_modules)
        {
            debug::log("Unloading module ", fst, " ", index, " of ", loaded_modules.size(), "\n");
            snd.unload();
        }

        debug::log("Stage 2: saving history...\n");
        std::ofstream ohfile(history_file, std::ios::out | std::ios::app);
        if (ohfile.is_open())
        {
            for (const auto & hist : history_lines) {
                ohfile << hist << std::endl;
            }

            ohfile.close();
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
               "list_modules [Module]       list modules\n"
               "load_config [Config Path]   load config\n"
               "show_config                 show config\n"
               );
        }
        else if (args.at(0) == "exit")
        {
            debug::log("Console requested termination. Exiting...\n");
            GlobalEventProcessor(GLOBAL_INSTANCE_NAME, GLOBAL_DESTROY_METHOD_NAME)();
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

            const std::string& module_path = args.at(1);
            const std::string module_name = path_to_name(args.at(1));

            if (loaded_modules.contains(module_name)) {
                debug::log("Module already loaded\n");
                return;
            }

            debug::log("Loading module: ", module_path, "...\n");
            try {
                Module module(module_path);
                auto deps = std::any_cast<std::vector<std::string>>(
                    module.call<std::vector<std::string>>("module_dependencies"));

                for (const auto & dep : deps)
                {
                    if (!loaded_modules.contains(dep)) {
                        debug::log("Module dependency ", dep, " missing!\n");
                        module.close_only();
                        return;
                    }
                }

                loaded_modules.emplace(module_name, module);
                module.init();
                debug::log("Module '", module_name, "' loaded.\n");
            } catch (const LibraryLoadError & err) {
                if (debug::verbose) {
                    debug::log("Library load error:\n", err.what());
                } else {
                    debug::log("Library load error!\nUse verbose mode to see trace info.\n");
                }
            } catch (const ModuleResolutionError & err) {
                if (debug::verbose) {
                    debug::log("Library load error:\n", err.what());
                } else {
                    debug::log("Library load error! Missing definition!\nUse verbose mode to see trace info.\n");
                }
            } catch (const SysdarftBaseError & err) {
                if (debug::verbose) {
                    debug::log("Library load error:\n", err.what());
                } else {
                    debug::log("Unknown error encountered during loading module!\n"
                        "Use verbose mode to see trace info.\n");
                }
            }
        }
        else if (args.at(0) == "unload_module")
        {
            if (args.size() != 2) {
                debug::log("unload_module [Module]\n");
                return;
            }

            if (!loaded_modules.contains(args.at(1))) {
                debug::log("Module not found\n");
                return;
            }

            // dependency check
            for (auto & module : loaded_modules)
            {
                auto deps = std::any_cast<std::vector < std::string >>(
                    module.second.call<std::vector<std::string>>("module_dependencies"));

                for (const auto & dep : deps)
                {
                    if (dep == args.at(1)) { // at least this module depends on module pending to unload
                        std::cout << "At least " << module.first << " requires " << args.at(1) << "!" << std::endl;
                        std::cout << "use `list_modules` to see the detailed dependencies." << std::endl;
                        std::cout << "Module not unloaded!" << std::endl;
                        return;
                    }
                }
            }

            Module module = loaded_modules.at(args.at(1));

            debug::log("Unloading module: ", args.at(1), "\n");
            try {
                module.unload();
                loaded_modules.erase(args.at(1));
            } catch (const SysdarftBaseError & err) {
                debug::log("Error encountered during loading module:\n", err.what(), "\n");
            }
        }
        else if (args.at(0) == "list_modules")
        {
            debug::log("A total of ", loaded_modules.size(), " module(s) loaded.\n");

            uint32_t index = 0;

            for (auto& module : loaded_modules)
            {
                std::stringstream index_str;
                index_str << "#" << index++;

                // show name
                debug::log(index_str.str(), ": ", module.first, "\n");

                // show dependencies
                auto deps = std::any_cast<std::vector<std::string>>(
                    module.second.call<std::vector<std::string>>("module_dependencies"));
                for (auto it = deps.begin(); it != deps.end(); ++it)
                {
                    std::string empty(index_str.str().size(), ' ');
                    std::string prefix = empty + "  ├── ";

                    if (it == deps.end() - 1) {
                        prefix = empty + "  └── ";
                    }

                    debug::log(prefix, *it, "\n");
                }
            }
        }
        else if (args.at(0) == "load_config")
        {
            if (args.size() != 2) {
                debug::log("load_config [Config Path]\n");
                return;
            }

            try {
                auto conf = load_config(args.at(1));
                GlobalEventProcessor.invoke_instance(
                    GLOBAL_INSTANCE_NAME,
                    GLOBAL_SET_CONFIG_METHOD_NAME,
                    { conf }
                );
            }
            catch (const ConfigError & err)
            {
                if (debug::verbose) {
                    debug::log("Configuration error:\n", err.what(), "\n");
                    return;
                }

                std::cout << "Error encountered when loading the configuration" << std::endl;
            } catch (const std::exception & err) {
                std::cout << "Error:" << err.what() << std::endl;
            }
        }
        else if (args.at(0) == "show_config")
        {
            auto conf = std::any_cast<config_t>(GlobalEventProcessor.invoke_instance(
                GLOBAL_INSTANCE_NAME,
                GLOBAL_GET_CONFIG_METHOD_NAME, { }));

            debug::log("Configuration:\n");

            for (const auto &[fst, snd] : conf) {
                debug::log("Section: ", fst, "\n");
                for (const auto & [Key, Val] : snd) {
                    debug::log("         " , Key, " : ", Val, "\n");
                }
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
    // input
    GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &input_processor,
        GLOBAL_INPUT_METHOD_NAME, &input_processor_::process_input);
    // destroy
    GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &cleanup_handler,
        GLOBAL_DESTROY_METHOD_NAME, &cleanup_handler_::destroy);
    // get_config
    GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &GlobalConfig,
        GLOBAL_GET_CONFIG_METHOD_NAME, &GlobalConfig_::get_config);
    // set_config
    GlobalEventProcessor.install_instance(GLOBAL_INSTANCE_NAME, &GlobalConfig,
        GLOBAL_SET_CONFIG_METHOD_NAME, &GlobalConfig_::set_config);

    std::thread CliWorkThread(&Cli::run, this);
    CliWorkThread.detach();
}
