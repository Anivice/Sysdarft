/* SysdarftMain.cpp
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <algorithm>
#include <ranges>
#include <string_view>
#include <SysdarftMain.h>
#include <SysdarftDebug.h>
#include <resources.h>

extern "C" void printLogo();

/**
 * fast_basename - Returns the basename portion of the given file path.
 *
 * @param path: The file path string.
 *
 * This function scans the string for directory separators ('/' and '\')
 * and returns a pointer to the character immediately following the last
 * separator. If no separator is found, it returns the original string.
 */
const char *fast_basename(const char *path)
{
    const char *base = path;

    if (!path) {
        return nullptr;
    }

    for (; *path; ++path) {
        if (*path == '/' || *path == '\\') {
            base = path + 1;
        }
    }
    return base;
}

void print_help(const char *program_name)
{
    std::stringstream ss;
    ss << "Usage: " << fast_basename(program_name) << " [OPTIONS]\n"
       << "Options:\n";

    uint64_t max_length_of_arguments = 0;
    for (long unsigned int i = 0; i < std::size(long_options) - 1; i++)
    {
        const auto& opt = long_options[i];
        auto this_len = std::strlen(opt.name) + 1 /* val */ + 5 /* `-[val], --[name]`, 5 additional characters */;
        max_length_of_arguments = std::max(this_len, max_length_of_arguments);
    }

    const auto before_explanation = max_length_of_arguments + 10 + 4;

    // Dynamically generate help text for each option
    for (long unsigned int i = 0; i < std::size(long_options) - 1; i++)
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
    printLogo();
    std::cout
        << "Sysdarft 64bit Hypothetical Architecture Version " << SYSDARFT_VERSION << std::endl
        << SYSDARFT_INFORMATION << std::endl;
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

uint64_t boot_sysdarft(
    const uint64_t memory_size,
    const std::string & font_name,
    const std::string & bios,
    const std::string & hdd,
    const std::string & fda,
    const std::string & fdb,
    const bool debug,
    const std::string & ip,
    const uint16_t port,
    const std::string & log_path,
    const bool headless,
    const bool gui,
    const bool cr_to_lf)
{
    std::ifstream file(bios, std::ios::in | std::ios::binary);
    std::vector<uint8_t> bios_code;
    const auto file_size = std::filesystem::file_size(bios);
    uint64_t ret;

    // read
    if (!file.is_open()) {
        throw SysdarftDisassemblerError("Could not open file " + bios);
    }

    bios_code.resize(file_size);
    file.read((char*)(bios_code.data()), static_cast<ssize_t>(file_size));
    if (static_cast<uint64_t>(file.gcount()) != file_size) {
        throw SysdarftDisassemblerError("Short read on file " + bios);
    }

    file.close();

    SysdarftCPU CPUInstance(memory_size, font_name, bios_code, hdd, fda, fdb);

    std::unique_ptr < RemoteDebugServer > debug_server;

    try {
        if (debug) { // request a debug server
            debug_server = std::make_unique<RemoteDebugServer>(ip, port, CPUInstance, log_path);
        }

        CPUInstance.translate_cr_to_lf = cr_to_lf;

        ret = CPUInstance.Boot(headless, gui);
    } catch (...) {
        throw;
    }

    return ret;
}

int main(int argc, char** argv)
{
    try
    {
        std::vector<std::unique_ptr<SysdarftModule>> loaded_modules;
        auto gnu_long_options = std::make_unique < option[] > (std::size(long_options));
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

        auto split = [](std::string_view input, char delimiter)->std::vector<std::string>
        {
            std::vector<std::string> parts;
            // Create a split view on the input based on the delimiter
            auto view = std::views::split(input, delimiter);

            // For each sub-range produced by the view, convert it to a string
            for (auto subrange : view) {
                parts.emplace_back(subrange.begin(), subrange.end());
            }

            return parts;
        };

        // Handle --verbose option
        if (parsed_options.contains("verbose")) {
            debug::verbose = true;
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

                if (src_files.empty()) {
                    std::cerr << "ERROR: No input files specified!" << std::endl;
                    exit_failure_on_error();
                }

                bool regex = false;
                if (parsed_options.contains("regex")) {
                    regex = true;
                }

                std::vector < std::string > include_path;
                if (parsed_options.contains("include")) {
                    include_path = parsed_options["include"];
                }

                if (format.at(0) == "bin") {
                    compile_to_binary(src_files, output_file.at(0), regex, BIN, include_path);
                } else if (format.at(0) == "exe") {
                    compile_to_binary(src_files, output_file.at(0), regex, EXE, include_path);
                } else if (format.at(0) == "sys") {
                    compile_to_binary(src_files, output_file.at(0), regex, SYS, include_path);
                } else {
                    exit_failure_on_error();
                }
            } catch (std::out_of_range &) {
                std::cerr << "ERROR: missing format, input file, or output file!" << std::endl;
                exit_failure_on_error();
            } catch (std::exception & e) {
                std::cerr << e.what() << std::endl;
                return EXIT_FAILURE;
            }

            return EXIT_SUCCESS;
        }

        if (parsed_options.contains("disassem"))
        {
            try {
                const auto src_files = parsed_options["disassem"];
                if (src_files.empty()) {
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

                std::string format;

                if (parsed_options.contains("format")) {
                    format = parsed_options["format"].at(0);
                } else {
                    format = "bin";
                }

                for (const auto & src_file : src_files)
                {
                    if (format == "bin") {
                        disassemble(src_file, origin, BIN);
                    } else if (format == "exe") {
                        disassemble(src_file, origin, EXE);
                    } else if (format == "sys") {
                        disassemble(src_file, origin, SYS);
                    } else {
                        std::cerr << "ERROR: Unknown format!" << std::endl;
                        exit_failure_on_error();
                    }
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

            uint64_t memory_size = 32 * 1024 * 1024;
            if (parsed_options.contains("memory")) {
                memory_size = std::strtoll(parsed_options["memory"].at(0).c_str(), nullptr, 10);
                memory_size *= 1024 * 1024; // 1MB
            }

            std::string hdd;
            if (parsed_options.contains("hdd")) {
                hdd = parsed_options["hdd"].at(0);
            }

            std::string fda;
            if (parsed_options.contains("fda")) {
                fda = parsed_options["fda"].at(0);
            }

            std::string fdb;
            if (parsed_options.contains("fdb")) {
                fdb = parsed_options["fdb"].at(0);
            }

            const bool headless = parsed_options.contains("no-curses");
            const bool gui = parsed_options.contains("with-gui");

            bool debug = false;
            std::string ip{};
            uint64_t port = 0;
            std::string log_file{};
            if (parsed_options.contains("debug"))
            {
                debug = true;
                auto ip_with_port = parsed_options["debug"].at(0);
                std::vector < std::string > ip_and_port;

                ip_and_port = split(ip_with_port, ':');
                if (ip_and_port.size() != 2)
                {
                    std::cerr << "ERROR: Incorrect target format provided!" << std::endl;
                    exit_failure_on_error();
                }

                ip = ip_and_port[0];
                const std::string port_literal = ip_and_port[1];
                port = std::strtoll(port_literal.c_str(), nullptr, 10);

                if (parsed_options.contains("crow-log")) {
                    log_file = parsed_options["crow-log"].at(0);
                }
            }

            std::string font = "JetBrainsMono_Medium";
            if (parsed_options.contains("font")) {
                font = parsed_options["font"].at(0);
            }

            if (font == "help")
            {
                std::cout << "Font can be an external file, or the following built-in fonts:" << std::endl;

                for (auto & i : font_list_string_table) {
                    std::cout << "    " << i + strlen("fonts_") << std::endl;
                }

                std::cout << std::endl;
                return EXIT_SUCCESS;
            }

            const bool cr_to_lf = parsed_options.contains("cr-to-lf");

            // boot system
            return static_cast<int>(boot_sysdarft(
                memory_size,
                font,
                bios_path,
                hdd,
                fda,
                fdb,
                debug,
                ip,
                port,
                log_file,
                headless,
                gui,
                cr_to_lf));
        }

        std::cout   << "If you see this message, that means you have provided one or more arguments,\n"
                    << "but none of them triggered any meaningful actions.\n"
                    << "Please refer to the help messages shown below to correct your arguments."
                    << std::endl;
        print_help(argv[0]);
        // print_version();
        return EXIT_FAILURE;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Argument parsing error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
#ifndef __DEBUG__
    catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        return EXIT_FAILURE;
    }
#endif //!__DEBUG__
}
