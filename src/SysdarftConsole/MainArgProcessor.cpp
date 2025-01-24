/* MainArgProcessor.cpp
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

#include <SysdarftMain.h>

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
                    parsed_options[opt_name].emplace_back(optarg ? optarg : "");
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

