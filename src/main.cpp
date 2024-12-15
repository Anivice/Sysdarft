#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <getopt.h>
#include <cstdlib>
#include <debug.h>
#include <cli.h>
#include <thread>

std::pair < std::map<std::string, std::string>, std::vector<std::string> >
get_args(int argc, char **argv, struct option long_options[])
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

    // A map to store the option name -> value
    std::map<std::string, std::string> parsed_options;

    int option;
    while ((option = getopt_long(argc, argv, short_options.c_str(), long_options, nullptr)) != -1)
    {
        if (option == '?')
        {
            // Unknown or invalid option
            std::cerr << "Unknown or invalid option encountered.\n";
            exit(EXIT_FAILURE);
        }
        else
        {
            // Find which long option this corresponds to
            for (int idx = 0; long_options[idx].name != nullptr; ++idx)
            {
                if (long_options[idx].val == option) {
                    // Found the matching option entry
                    std::string opt_name(long_options[idx].name);
                    const std::string opt_value = optarg ? optarg : "";
                    parsed_options[opt_name] = opt_value;
                    break;
                }
            }
        }
    }

    // Collect non-option arguments
    for (int i = optind; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    std::pair pair(parsed_options, args);
    return pair;
}

void print_help(const char * program_name)
{
    std::cout
    << "Usage: " << program_name << " [OPTIONS]\n"
    << "Options:\n"
    << "    -h, --help        Show this help message\n"
    << "    -v, --version     Show version information\n"
    << "    -c, --config      Load a configuration file\n"
    << "    -V, --verbose     Enable verbose mode. Additional debug message will be printed\n"
    << std::endl;
}

void print_version()
{
    std::cout
    << "Sysdarft 64bit Hypothetical Architecture Version" << SYSDARFT_VERSION << std::endl
    << SYSDARFT_INFORMATION << std::endl;
}



int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"help",        no_argument,       nullptr, 'h'},
        {"version",     no_argument,       nullptr, 'v'},
        {"config",      no_argument,       nullptr, 'c'},
        {"verbose",     no_argument,       nullptr, 'V'},
        {nullptr,       0,                 nullptr,  0 }
    };

    const auto args = get_args(argc, argv, long_options);

    if (args.first.contains("help")) {
        print_help(argv[0]);
        return 0;
    }

    if (args.first.contains("version")) {
        print_version();
        return 0;
    }

    if (args.first.contains("verbose")) {
        debug::verbose = true;
    }

    Cli screen;

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(3600));
    }

    return EXIT_SUCCESS;
}
