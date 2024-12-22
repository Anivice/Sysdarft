#include <instruction.h>
#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <cstdlib>
#include <thread>
#include <stdexcept>
#include <fstream>
#include <sstream>

using ParsedArgs = std::pair<std::map<std::string, std::string>, std::vector<std::string>>;
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

void print_help(const char *program_name)
{
    std::cout
        << "Usage: " << program_name << " [OPTIONS] [INPUT FILES]\n"
        << "Options:\n"
        << "    -h, --help        Show this help message\n"
        << "    -v, --version     Show version information\n"
        << "    -o, --output      Output file\n"
        << "    -i, --input       Input file\n"
        << std::endl;
}

void print_version()
{
    std::cout
        << "Sysdarft 64bit Hypothetical Architecture Version " << SYSDARFT_VERSION << std::endl
        << SYSDARFT_INFORMATION << std::endl;
}

std::string toLowerCase(const std::string& input)
{
    std::string output = input; // Copy the original string
    std::transform(output.begin(), output.end(), output.begin(),
                   [](const unsigned char c) { return std::tolower(c); });
    return output;
}

std::string process_line(const std::vector<std::string>& asm_line) {
    std::stringstream ret;
    size_t i = 0;

    // Helper to parse individual instruction arguments
    auto instruction_parse = [&]() -> std::string {
        std::stringstream iret;

        if (i >= asm_line.size()) {
            throw std::runtime_error("Unexpected end of line while parsing.");
        }

        if (asm_line[i] == "*") { // Memory address
            iret << asm_line[i++];

            if (i < asm_line.size() && asm_line[i] == "(") { // Ignored ratio
                iret << "1";
            } else if (i < asm_line.size()) { // Ratio
                iret << asm_line[i++];
            } else {
                throw std::runtime_error("Syntax error: unexpected end of line after memory address.");
            }

            if (i >= asm_line.size() || asm_line[i] != "(") {
                throw std::runtime_error("Syntax error: expected '(', found: " + (i < asm_line.size() ? asm_line[i] : "<end of line>"));
            }

            iret << "(";
            i++;
            while (i < asm_line.size() && asm_line[i] != ")") {
                iret << asm_line[i++];
            }

            if (i >= asm_line.size() || asm_line[i] != ")") {
                throw std::runtime_error("Syntax error: expected ')', found: " + (i < asm_line.size() ? asm_line[i] : "<end of line>"));
            }

            iret << ")";
            i++;
        } else if (asm_line[i] == "$" || asm_line[i] == "%") { // Constants or Registers
            while (i < asm_line.size() && asm_line[i] != ",") {
                iret << asm_line[i++];
            }
        } else { // Invalid syntax
            throw std::runtime_error("Invalid syntax: " + asm_line[i]);
        }

        return iret.str();
    };

    while (i < asm_line.size()) {
        // Handle line marks
        if (i + 1 < asm_line.size() && asm_line[i + 1] == ":") {
            ret << asm_line[i] << ":";
            i += 2;
            continue;
        }

        if (i >= asm_line.size()) break;

        // Handle instructions
        const auto& uppercased_input = toUpperCaseTransform(asm_line[i]);
        if (instructionMap.contains(uppercased_input)) {
            ret << toLowerCase(asm_line[i++]);
            const int argc = instructionMap.at(uppercased_input).at("argc");

            for (int arg_count = 0; arg_count < argc; ++arg_count) {
                if (i < asm_line.size()) {
                    ret << instruction_parse();
                } else {
                    throw std::runtime_error("Missing arguments for instruction: " + uppercased_input);
                }
            }
        } else {
            throw std::runtime_error("Unknown instruction: " + asm_line[i]);
        }
    }

    return ret.str();
}

int main(int argc, char** argv)
{
    static struct option long_options[] = {
        {"help",    no_argument,       nullptr, 'h'},
        {"version", no_argument,       nullptr, 'v'},
        {"output",  required_argument, nullptr, 'o'},
        {"input",   required_argument, nullptr, 'i'},
        {nullptr,   0,                 nullptr,  0 }
    };

    try {
        const auto & [parsed_options, src_files] = get_args(argc, argv, long_options);

        // Handle --help option
        if (parsed_options.contains("help")) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        // Handle --version option
        if (parsed_options.contains("version")) {
            print_help(argv[0]);
            return EXIT_SUCCESS;
        }

        if (!parsed_options.contains("output") || !parsed_options.contains("input"))
        {
            std::cerr << "You must specify an input file and a output file." << std::endl;
            print_help(argv[0]);
            return EXIT_FAILURE;
        }

        std::string ofile = parsed_options.at("output");
        std::string ifile = parsed_options.at("input");
        std::ifstream ifile_stream(ifile, std::ios::in);
        std::ofstream ofile_stream(ofile, std::ios::out);


        if (!ifile_stream.is_open()) {
            std::cerr << "Error opening file " << ifile << std::endl;
            return EXIT_FAILURE;
        }

        if (!ofile_stream.is_open()) {
            std::cerr << "Error opening file " << ofile << std::endl;
            return EXIT_FAILURE;
        }

        std::string infile_buffer;
        while (!ifile_stream.eof())
        {
            infile_buffer.clear();
            getline(ifile_stream, infile_buffer);
            infile_buffer = replaceAsterisks(infile_buffer, "*", " * ");
            infile_buffer = replaceAsterisks(infile_buffer, ":", " : ");
            infile_buffer = replaceAsterisks(infile_buffer, "%", " % ");
            infile_buffer = replaceAsterisks(infile_buffer, "$", " $ ");
            infile_buffer = replaceAsterisks(infile_buffer, "(", " ( ");
            infile_buffer = replaceAsterisks(infile_buffer, ")", " ) ");
            infile_buffer = replaceAsterisks(infile_buffer, ",", " , ");
            const auto & before_process =
                lines_to_words(infile_buffer, { ' ' });

            ofile_stream << process_line(before_process) << std::endl;
        }

        ifile_stream.close();
        ofile_stream.close();

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
