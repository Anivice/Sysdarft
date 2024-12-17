#include <module.h>
#include <cli.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <debug.h>
#include <ui_curses.h>
#include <csignal>
#include <fstream>
#include <thread>
#include <atomic>
#include <map>
#include <unordered_map>
#include <filesystem>
#include "cli_local.h"
#include "ModuleForest.h"

std::unordered_map < std::string, Module > loaded_modules;
std::map < std::string, std::vector < std::string > > module_list_reference;
std::atomic<bool> gSigintReceived(false);
std::string prefix = "sysdarft> ";
std::string history_file;
std::vector<std::string> history_lines;

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
    set_thread_name("Sysdarft Console");
    // Setup sigaction for SIGINT
    struct sigaction sa{};
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

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
            debug::log(" -- Unloading module ", fst, " ", index++, " of ", loaded_modules.size(), "\n");
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

        debug::log("Stage 3: Cleaning up UI...\n");
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_CLEANUP_METHOD_NAME)();

        debug::log("All stages completed!\n");
        GlobalEventNotifier = GLOBAL_QUIT_EVENT;
    }
} cleanup_handler;

class input_processor_ {
public:
    void do_help()
    {
        debug::log(
            "help                        print this help menu\n"
            "exit                        exit program\n"
            "load_module [Module Path]   load module\n"
            "unload_module [Module]      load module\n"
            "list_modules [Module]       list modules\n"
            "load_config [Config Path]   load config\n"
            "show_config                 show config\n"
            "test_curses                 test functionality of curses and then exit\n"
            "ls                          list entries under current directory\n");
    }

    void do_exit()
    {
        debug::log("Console requested termination. Exiting...\n");
        GlobalEventProcessor(GLOBAL_INSTANCE_NAME, GLOBAL_DESTROY_METHOD_NAME)();
    }

    void do_load_module(const std::vector < std::string > & args)
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

            module.init();
            module.disable_delete();
            loaded_modules.emplace(module_name, module);
            module_list_reference.emplace(module_name, deps);
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

    void do_unload_module(const std::vector < std::string > & args)
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
            const auto & deps = module_list_reference.at(module.first);

            for (const auto & dep : deps)
            {
                if (dep == args.at(1)) { // at least this module depends on module pending to unload
                    debug::log("At least ", module.first, " requires ", args.at(1), "!\n");
                    debug::log("use `list_modules` to see the detailed dependencies.\nModule not unloaded!\n");
                    return;
                }
            }
        }

        Module module = loaded_modules.at(args.at(1));

        debug::log("Unloading module: ", args.at(1), "\n");
        try {
            module.unload();
            loaded_modules.erase(args.at(1));
            module_list_reference.erase(args.at(1));
        } catch (const SysdarftBaseError & err) {
            debug::log("Error encountered during loading module:\n", err.what(), "\n");
        }
    }

    void do_list_modules()
    {
        debug::log("A total of ", loaded_modules.size(), " module(s) loaded.\n");
        ModuleForest_ forest(module_list_reference);
        forest.printForest();
    }

    void do_load_config(const std::vector < std::string > & args)
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

            debug::log("Error encountered when loading the configuration.\n");
        } catch (const std::exception & err) {
            debug::log("Error:", err.what(), "\n");
        }
    }

    void do_show_config()
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

    void do_test_curses()
    {
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_INITIALIZE_METHOD_NAME)();
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_SET_CURSOR_VISIBILITY_METHOD_NAME)(true);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        GlobalEventProcessor(UI_INSTANCE_NAME, UI_CLEANUP_METHOD_NAME)();
        // GlobalEventProcessor(GLOBAL_INSTANCE_NAME, GLOBAL_DESTROY_METHOD_NAME)();
    }

    void do_ls()
    {
        std::string path = ".";
        std::stringstream ss;
        try
        {
            for(const auto& entry : std::filesystem::directory_iterator(path))
            {
                ss << entry.path().filename().string();
                if(entry.is_directory()) {
                    ss << '/';
                }
                ss << "\n";
            }

            debug::log(ss.str());
        } catch(const std::exception & err) {
            debug::log("Error: ", err.what(), '\n');
        }
    }

    void process_input(const std::vector < std::string > & args)
    {
        if (args.empty()) {
            return;
        }

        if (args.at(0) == "help") {
            do_help();
        } else if (args.at(0) == "exit") {
            do_exit();
        } else if (args.at(0) == "load_module") {
            do_load_module(args);
        } else if (args.at(0) == "unload_module") {
            do_unload_module(args);
        } else if (args.at(0) == "list_modules") {
            do_list_modules();
        } else if (args.at(0) == "load_config") {
            do_load_config(args);
        } else if (args.at(0) == "show_config") {
            do_show_config();
        } else if (args.at(0) == "test_curses") {
            do_test_curses();
        } else if (args.at(0) == "ls") {
            do_ls();
        }
        else
        {
            debug::log("Unknown command: ", args.at(0), "\n");
            debug::log("Use `help` to show the usage.\n");
        }
    }
} input_processor;

class dummy_ {
public:
    void dummy_input_handler(int) {
        return;
    }
} dummy;

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

    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &dummy,
        UI_INPUT_MONITOR_METHOD_NAME, &dummy_::dummy_input_handler);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_CLEANUP_METHOD_NAME, &ui_curses::cleanup);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_INITIALIZE_METHOD_NAME, &ui_curses::initialize);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_METHOD_NAME, &ui_curses::set_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_GET_CURSOR_METHOD_NAME, &ui_curses::get_cursor);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_DISPLAY_CHAR_METHOD_NAME, &ui_curses::display_char);
    GlobalEventProcessor.install_instance(UI_INSTANCE_NAME, &curses,
        UI_SET_CURSOR_VISIBILITY_METHOD_NAME, &ui_curses::set_cursor_visibility);

    std::thread CliWorkThread(&Cli::run, this);
    CliWorkThread.detach();
}
