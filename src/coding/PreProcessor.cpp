#include <regex>
#include <string>
#include <EncodingDecoding.h>

const std::regex pre_processor_pattern(R"(\.(EQU|equ|ORG|org|LAB|lab)\s+<(.*)\|(.*)>)");

std::string ProProcessor(std::basic_iostream<char>& file,
        uint64_t & org,
        std::map < std::string, std::pair < uint64_t /* line position */,
            std::vector < uint64_t > > > & defined_line_marker)
{
    auto replace_all = [](std::string & input,
        const std::string & target,
        const std::string & replacement)
    {
        if (target.empty()) return; // Avoid infinite loop if target is empty

        size_t pos = 0;
        while ((pos = input.find(target, pos)) != std::string::npos) {
            input.replace(pos, target.length(), replacement);
            pos += replacement.length(); // Move past the replacement to avoid infinite loop
        }
    };

    std::map < std::string, std::string > equ_table;
    std::string line;
    std::stringstream code;

    while (std::getline(file, line))
    {
        if (std::smatch matches;
            std::regex_match(line, matches, pre_processor_pattern))
        {
            if (matches[1] == "EQU" || matches[1] == "equ")
            {
                if (matches.size() != 4) {
                    throw SysdarftAssemblerError("Syntax error in EQU definition: " + line);
                }

                auto entry = matches[2].str();
                auto value = matches[3].str();
                replace_all(entry, " ", "");
                replace_all(value, " ", "");

                equ_table[matches[2]] = matches[3];
            }
            else if (matches[1] == "ORG" || matches[1] == "org")
            {
                if (matches.size() != 3) {
                    throw SysdarftAssemblerError("Syntax error in ORG definition: " + line);
                }

                org = std::stoull(matches[2]);
            }
            else if (matches[1] == "LAB" || matches[1] == "lab")
            {
                if (matches.size() != 3) {
                    throw SysdarftAssemblerError("Syntax error in LAB definition: " + line);
                }

                defined_line_marker.emplace(matches[2], std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
            }
            else {
                throw SysdarftAssemblerError("Unknown preprocessing sign: " + line);
            }
        }
        else
        {
            code << line + "\n";
        }
    }

    auto content = code.str();

    for (const auto &[fst, snd] : equ_table)
    {
        std::stringstream args;
        args << "s/" << fst << "/" << snd << "/";

        auto [fd_stdout, fd_stderr, exit_status] =
            debug::exec_command("/usr/bin/sed", content, args.str(), "-E");
        if (exit_status != 0) {
            throw SysdarftAssemblerError("Syntax error in EQU definition: " + fd_stderr);
        }

        content = fd_stdout;
    }

    return content;
}
