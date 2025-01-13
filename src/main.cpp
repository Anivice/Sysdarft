#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <getopt.h>
#include <stdexcept>
#include <debug.h>
#include <module.h>

// Each option name maps to a list of string values.
using ParsedOptions = std::map<std::string, std::vector<std::string>>;

// The return type includes:
//   1) A map from option-name -> vector of values
//   2) A vector of positional arguments (non-option arguments).
using ParsedArgs = std::pair<ParsedOptions, std::vector<std::string>>;

ParsedArgs get_args(int argc, char** argv, struct option long_options[])
{
    // Build the short options string from the long_options array
    std::string short_opts;
    for (int idx = 0; long_options[idx].name != nullptr; ++idx) {
        if (long_options[idx].val != 0) {
            short_opts.push_back(static_cast<char>(long_options[idx].val));
            if (long_options[idx].has_arg == required_argument) {
                short_opts.push_back(':');
            } else if (long_options[idx].has_arg == optional_argument) {
                short_opts.append("::");
            }
        }
    }

    ParsedOptions parsed_options;
    std::vector<std::string> positional_args;

    // Parse
    while (true) {
        int opt_index = 0;
        int option = getopt_long(argc, argv, short_opts.c_str(), long_options, &opt_index);
        if (option == -1) {
            break; // No more options
        }
        if (option == '?') {
            // Unknown or invalid option
            throw std::invalid_argument("Unknown or invalid option encountered.");
        } else {
            // Find which long option was matched
            for (int idx = 0; long_options[idx].name != nullptr; ++idx) {
                if (long_options[idx].val == option) {
                    const std::string opt_name(long_options[idx].name);
                    // If this option appears multiple times,
                    // push_back each new value instead of overwriting.
                    parsed_options[opt_name].push_back(optarg ? optarg : "");
                    break;
                }
            }
        }
    }

    // Collect non-option (positional) arguments
    for (int i = optind; i < argc; ++i) {
        positional_args.emplace_back(argv[i]);
    }

    return {parsed_options, positional_args};
}

void print_help(const char *program_name)
{
    std::cout
        << "Usage: " << program_name << " [OPTIONS]\n"
        << "Options:\n"
        << "    -h, --help        Show this help message\n"
        << "    -v, --version     Show version information\n"
        << "    -m, --module      Load a configuration file\n"
        << "    -V, --verbose     Enable verbose mode. Additional debug messages will be printed\n"
        << std::endl;
}

void print_version()
{
    std::cout
        << "Sysdarft 64bit Hypothetical Architecture Version " << SYSDARFT_VERSION << std::endl
        << SYSDARFT_INFORMATION << std::endl;
}

int main(int argc, char** argv)
{
    set_thread_name("Sysdarft Watcher");
    std::vector < std::unique_ptr<Module> > modules;

    static struct option long_options[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"version", no_argument,       nullptr, 'v'},
        {"module",  required_argument, nullptr, 'm'},
        {"verbose", no_argument,       nullptr, 'V'},
        {nullptr,   0,                 nullptr,  0 }
    };

    try {
        auto [parsed_options, positional_args] = get_args(argc, argv, long_options);

        if (parsed_options.empty() && positional_args.empty())
        {
            log("Not enough arguments.\n");
            print_help(argv[0]);
            return EXIT_FAILURE;
        }

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
            log("Verbose mode enabled.\n");
        }

        // Handle --module option
        if (parsed_options.contains("module"))
        {
            const auto module_path = parsed_options["module"];
            for (const auto & path : module_path)
            {
                modules.emplace_back(std::make_unique<Module>(path));
            }
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
