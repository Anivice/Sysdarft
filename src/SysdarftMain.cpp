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
#include <SysdarftDebug.h>
#include <SysdarftModule.h>
#include <EncodingDecoding.h>

struct option_complicated
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
    const char * arg_explain;
};

static option_complicated long_options[] = {
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
    {"boot",    no_argument,        nullptr, 'S',   "Boot the system"},
    {nullptr,   0,                  nullptr,  0,    nullptr }
};

// Each option name maps to a list of string values.
using ParsedOptions = std::map<std::string, std::vector<std::string>>;

// The return type includes:
//   1) A map from option-name -> vector of values
//   2) A vector of positional arguments (non-option arguments).
using ParsedArgs = std::pair<ParsedOptions, std::vector<std::string>>;

ParsedArgs get_args(const int argc, char** argv, option long_options[])
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
    while (true)
    {
        int opt_index = 0;
        const int option = getopt_long(argc, argv, short_opts.c_str(), long_options, &opt_index);
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
    std::stringstream ss;
    ss << "Usage: " << program_name << " [OPTIONS]\n"
       << "Options:\n";

    uint64_t max_length_of_arguments = 0;
    for (int i = 0; i < std::size(long_options) - 1; i++)
    {
        const auto& opt = long_options[i];
        auto this_len = std::strlen(opt.name) + 1 /* val */ + 5 /* `-[val], --[name]`, 5 additional characters */;
        max_length_of_arguments = std::max(this_len, max_length_of_arguments);
    }

    const auto before_explanation = max_length_of_arguments + 10 + 4;

    // Dynamically generate help text for each option
    for (int i = 0; i < std::size(long_options) - 1; i++)
    {
        const auto& opt = long_options[i];
        std::stringstream this_line;
        this_line << "    ";

        // Print short option if provided
        if (opt.val) {
            this_line << "-" << static_cast<char>(opt.val) << ", ";
        } else {
            this_line << "    ";  // Indent if no short option
        }

        // Print long option
        this_line << "--" << opt.name;

        // If option requires an argument, show placeholder
        if (opt.has_arg) {
            this_line << " <arg>";
        }

        const auto printed_len = this_line.str().length();
        const auto space = before_explanation - printed_len;
        const std::string padding(space, ' ');
        const std::string next_line_padding(before_explanation, ' ');
        this_line << padding;

        std::string exp = opt.arg_explain;
        replace_all(exp, "\n", "\n    " + next_line_padding);
        this_line << exp;

        ss << this_line.str() << std::endl;
    }

    std::cout << ss.str();
}

void print_version()
{
    std::cout
        << "Sysdarft 64bit Hypothetical Architecture Version " << SYSDARFT_VERSION << std::endl
        << SYSDARFT_INFORMATION << std::endl;
}

void compile_to_binary(const std::vector< std::string > & source_files, const std::string & binary_filename)
{
    try {
        std::vector < std::vector < uint8_t > > binary_cct;
        for (const std::string & source_file : source_files)
        {
            std::fstream file(source_file, std::ios::in | std::ios::out);
            if (!file.is_open()) {
                throw SysdarftAssemblerError("Could not open file " + source_file);
            }

            std::vector < uint8_t > binary;
            CodeProcessing(binary, file);

            file.close();
            binary_cct.emplace_back(binary);
        }

        std::ofstream file(binary_filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw SysdarftAssemblerError("Could not open file " + binary_filename);
        }

        for (const auto & cct : binary_cct) {
            file.write(reinterpret_cast<const char*>(cct.data()), cct.size());
        }

        file.close();
    } catch (const std::exception & e) {
        throw SysdarftAssemblerError(e.what());
    }
}

class SysdarftDisassemblerError final : public SysdarftBaseError
{
public:
    explicit SysdarftDisassemblerError(const std::string & message) :
        SysdarftBaseError("Disassembler failed to process data: " + message) { }
};

void disassemble(const std::string & binary_filename, const uint64_t org)
{
    std::ifstream file(binary_filename, std::ios::in | std::ios::binary);
    std::vector < uint8_t > assembled_code;
    auto file_size = std::filesystem::file_size(binary_filename);

    // read
    if (!file.is_open()) {
        throw SysdarftDisassemblerError("Could not open file " + binary_filename);
    }

    assembled_code.resize(file_size);
    file.read((char*)(assembled_code.data()), file_size);
    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + binary_filename);
    }

    file.close();

    const auto space = assembled_code.size();
    std::vector < std::string > lines;
    while (!assembled_code.empty())
    {
        std::stringstream off;
        std::vector < std::string > line;
        off << std::hex << std::setfill('0') << std::setw(16) << std::uppercase
            << space - assembled_code.size() + org;

        decode_instruction(line, assembled_code);

        if (!line.empty()) {
            off << ": " << line[0];
        } else {
            off << ": " << "(bad)";
        }

        lines.push_back(off.str());
    }

    for (const auto& line : lines) {
        std::cout << line << "\n";
    }
}

void complicated_to_gnu(option * dest, const option_complicated * src)
{
    uint64_t offset = 0;
    while (src[offset].name)
    {
        dest[offset].name = src[offset].name;
        dest[offset].flag = src[offset].flag;
        dest[offset].has_arg = src[offset].has_arg;
        dest[offset].val = src[offset].val;
        offset++;
    }

    dest[offset] = {nullptr, 0, nullptr, 0 };
}

void boot_sysdarft(
    const std::string & bios,
    const std::string & hdd,
    const std::string & fda,
    const std::string & fdb)
{

}

int main(int argc, char** argv)
{
    try
    {
        std::vector<std::unique_ptr<SysdarftModule>> loaded_modules;
        std::unique_ptr < option[] > gnu_long_options = std::make_unique < option[] > (std::size(long_options));
        complicated_to_gnu(gnu_long_options.get(), long_options);
        auto [parsed_options, positional_args]
            = get_args(argc, argv, gnu_long_options.get());

        auto exit_failure_on_error = [&]()->void {
            std::cout << "Use -h or --help to see usage" << std::endl;
            exit(EXIT_FAILURE);
        };

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
            std::cerr << "Verbose mode enabled." << std::endl;
        }

        // Handle --module option
        if (parsed_options.contains("module"))
        {
            for (const auto module_path = parsed_options["module"];
                const auto & path : module_path)
            {
                loaded_modules.emplace_back(std::make_unique<SysdarftModule>(path));
            }
        }

        // compile file
        if (parsed_options.contains("compile"))
        {
            try
            {
                const auto output_file = parsed_options["output"];
                const auto format = parsed_options["format"];
                const auto src_files = parsed_options["compile"];
                const auto org_assembly = parsed_options["original"];

                if (format.size() != 1) {
                    std::cerr << "ERROR: Unknown compilation format!" << std::endl;
                    exit_failure_on_error();
                }

                if (output_file.size() != 1) {
                    std::cerr << "ERROR: No or multiple output file specified!" << std::endl;
                    exit_failure_on_error();
                }

                if (src_files.size() == 0) {
                    std::cerr << "ERROR: No input files specified!" << std::endl;
                    exit_failure_on_error();
                }

                if (format.at(0) == "bin") {
                    compile_to_binary(src_files, output_file.at(0));
                } else if (format.at(0) == "exe") {
                    std::cerr << "ERROR: Feature not implemented!" << std::endl;
                    exit_failure_on_error();
                } else if (format.at(0) == "sys") {
                    std::cerr << "ERROR: Feature not implemented!" << std::endl;
                    exit_failure_on_error();
                } else {
                    exit_failure_on_error();
                }
            } catch (std::out_of_range &) {
                std::cerr << "ERROR: missing format, input file, or output file!" << std::endl;
                exit_failure_on_error();
            } catch (SysdarftAssemblerError & e) {
                std::cerr << "ERROR: Error when compiling code:\n" << e.what() << std::endl;
            }

            return EXIT_SUCCESS;
        }

        if (parsed_options.contains("disassem"))
        {
            try {
                const auto src_files = parsed_options["disassem"];
                if (src_files.size() == 0) {
                    std::cerr << "ERROR: No input files specified!" << std::endl;
                    exit_failure_on_error();
                }

                // redefine origin
                uint64_t origin = 0;
                if (parsed_options.contains("origin")) {
                    auto expression = parsed_options["origin"][0];
                    process_base16(expression);
                    origin = std::strtoull(expression.c_str(), nullptr, 10);
                }

                const std::string line = "================================================================";

                for (const auto & src_file : src_files)
                {
                    const int len = (line.length() - (src_file.length() + 4)) / 2;
                    const int space_after = line.length() - len * 2 - 2 - src_file.length();
                    const std::string line_before_n_after(len, '=');
                    const std::string string_space_after(space_after, ' ');
                    std::cout << ";" << line << std::endl;
                    std::cout << ";" << line_before_n_after << "  "
                              << src_file << string_space_after << line_before_n_after << std::endl;
                    std::cout << ";" << line << std::endl;

                    disassemble(src_file, origin);

                    std::cout << ";" << line << std::endl;
                }

            } catch (std::out_of_range &) {
                std::cerr << "ERROR: Missing disassemble option!" << std::endl;
                exit_failure_on_error();
            } catch (SysdarftBaseError & e) {
                std::cerr << "ERROR: " << e.what() << std::endl;
                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }

        if (parsed_options.contains("boot"))
        {
            if (parsed_options["bios"].size() != 1) {
                std::cerr << "ERROR: Missing BIOS!" << std::endl;
                exit_failure_on_error();
            }

            const std::string bios_path = parsed_options["bios"][0];
            std::string hdd;
            std::string fda;
            std::string fdb;

            boot_sysdarft(bios_path, hdd, fda, fdb);
            return EXIT_SUCCESS;
        }

        print_help(argv[0]);
        print_version();
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
