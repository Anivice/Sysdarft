#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <getopt.h>
#include <cstdlib>
#include <debug.h>
#include <cli.h>
#include <thread>
#include <global.h>
#include <stdexcept>

/**
 * @brief Type alias for parsed command-line arguments.
 *
 * Defines a pair consisting of:
 * - A map of option names to their corresponding values.
 * - A vector of non-option arguments.
 */
using ParsedArgs = std::pair<std::map<std::string, std::string>, std::vector<std::string>>;

/**
 * @brief Parses command-line arguments using getopt_long.
 *
 * Dynamically constructs the short options string based on the provided long options array,
 * processes options, and collects non-option arguments.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @param long_options An array of `struct option` defining supported long options.
 *
 * @return A `ParsedArgs` pair containing:
 *         - A map of option names to their values.
 *         - A vector of positional (non-option) arguments.
 *
 * @throws std::invalid_argument If an unknown or invalid option is encountered.
 */
ParsedArgs get_args(int argc, char **argv, struct option long_options[])
{
    std::vector<std::string> args;

    // Dynamically build the short options string from long_options
    std::string short_options;
    for (int idx = 0; long_options[idx].name != nullptr; ++idx) {
        if (long_options[idx].val != 0) {
            short_options.push_back(static_cast<char>(long_options[idx].val));
            if (long_options[idx].has_arg == required_argument) {
                short_options.push_back(':');
            } else if (long_options[idx].has_arg == optional_argument) {
                short_options.append("::");
            }
        }
    }

    std::map<std::string, std::string> parsed_options;

    int option;
    while ((option = getopt_long(argc, argv, short_options.c_str(), long_options, nullptr)) != -1)
    {
        if (option == '?') {
            // Unknown or invalid option encountered
            throw std::invalid_argument("Unknown or invalid option encountered.");
        } else {
            // Match the option to its long option entry
            for (int idx = 0; long_options[idx].name != nullptr; ++idx) {
                if (long_options[idx].val == option) {
                    std::string opt_name(long_options[idx].name);
                    const std::string opt_value = optarg ? optarg : "";
                    parsed_options[opt_name] = opt_value;
                    break;
                }
            }
        }
    }

    // Collect non-option (positional) arguments
    for (int i = optind; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return std::make_pair(parsed_options, args);
}

/**
 * @brief Displays the help message for the program.
 *
 * @param program_name The name of the executable (usually `argv[0]`).
 */
void print_help(const char *program_name)
{
    std::cout
        << "Usage: " << program_name << " [OPTIONS]\n"
        << "Options:\n"
        << "    -h, --help        Show this help message\n"
        << "    -v, --version     Show version information\n"
        << "    -c, --config      Load a configuration file\n"
        << "    -V, --verbose     Enable verbose mode. Additional debug messages will be printed\n"
        << std::endl;
}

/**
 * @brief Displays the version information of the program.
 */
void print_version()
{
    std::cout
        << "Sysdarft 64bit Hypothetical Architecture Version " << SYSDARFT_VERSION << std::endl
        << SYSDARFT_INFORMATION << std::endl;
}

/**
 * @brief Entry point of the Sysdarft application.
 *
 * Initializes the program by parsing command-line arguments, configuring
 * the user interface, and entering the main event loop.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 *
 * @return EXIT_SUCCESS on successful execution, or an appropriate exit code on failure.
 */
int main(int argc, char** argv)
{
    set_thread_name("Sysdarft Watcher");

    static struct option long_options[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"version", no_argument,       nullptr, 'v'},
        {"config",  required_argument, nullptr, 'c'},
        {"verbose", no_argument,       nullptr, 'V'},
        {nullptr,   0,                 nullptr,  0 }
    };

    try {
        auto [parsed_options, positional_args] = get_args(argc, argv, long_options);

        // Handle --help option
        if (parsed_options.contains("help")) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        // Handle --version option
        if (parsed_options.contains("version")) {
            print_version();
            return EXIT_SUCCESS;
        }

        // Handle --verbose option
        if (parsed_options.contains("verbose")) {
            debug::verbose = true;
            debug::log("Verbose mode enabled.\n");
        }

        // Handle --config option
        if (parsed_options.contains("config")) {
            std::string config_path = parsed_options["config"];
            // TODO: Load configuration file
            // Example:
            // if (!screen.load_config(config_path)) {
            //     std::cerr << "Failed to load configuration file: " << config_path << std::endl;
            //     return EXIT_FAILURE;
            // }
        }

        // Initialize the CLI (Command-Line Interface) component
        Cli screen;

        // Main event loop: run until a GLOBAL_QUIT_EVENT is triggered
        while (GlobalEventNotifier != GLOBAL_QUIT_EVENT) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        return EXIT_SUCCESS;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Argument parsing error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return EXIT_FAILURE;
    }
}
