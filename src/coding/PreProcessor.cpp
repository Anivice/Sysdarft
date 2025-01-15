#include <regex>
#include <string>
#include <iomanip>
#include <EncodingDecoding.h>

const std::regex org_pattern(R"(\s*\.org\s+((?:0x[0-9A-Fa-f]+)|(?:\d+))\s*)", std::regex_constants::icase); // .org 0x123
const std::regex lab_pattern(R"(\s*\.lab\s+([A-Za-z._][A-Za-z0-9._]*(?:\s*,\s*[A-Za-z._][A-Za-z0-9._]*)*)\s*)",
    std::regex_constants::icase);
const std::regex equ_pattern(R"(\s*\.equ\s+'([^']*)'\s*,\s*'([^']*)'\s*)", std::regex_constants::icase);

std::vector<std::string> splitString(const std::string& input, const char delimiter = ',')
{
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

std::string convertEscapeSequences(const std::string& input)
{
    std::string output;
    output.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            // Check the character following the backslash
            switch (const char nextChar = input[i + 1]) {
            case 'n':
                output.push_back('\n');
                break;
            case 't':
                output.push_back('\t');
                break;
            case '\\':
                output.push_back('\\');
                break;
            case '"':
                output.push_back('\"');
                break;
                // Add additional escape sequence cases as needed
            default:
                // If unknown escape, keep the backslash and next char as is
                    output.push_back('\\');
                output.push_back(nextChar);
                break;
            }
            i++;  // Skip the next character because we've processed it
        } else {
            output.push_back(input[i]);
        }
    }

    return output;
}

std::string truncateAfterSemicolonOrHash(const std::string& input)
{
    // Find the first occurrence of either ';' or '#'
    // If found, return substring from beginning to that position
    if (const size_t pos = input.find_first_of(";#");
        pos != std::string::npos)
    {
        return input.substr(0, pos);
    }
    // If neither found, return the original string
    return input;
}

void process_org(const std::string& input, uint64_t & org)
{
    // .org [NUM]
    if (std::smatch matches; std::regex_search(input, matches, org_pattern))
    {
        auto num_literal = matches[1].str();
        process_base16(num_literal);
        org = std::strtoll(num_literal.c_str(), nullptr, 10);
    }
}

void process_lab(const std::string& input, defined_line_marker_t & defined_line_marker)
{
    // .lab marker1, [marker2, ...]
    if (std::smatch matches; std::regex_search(input, matches, lab_pattern))
    {
        auto marker_literal = matches[1].str();
        replace_all(marker_literal, " ", "");
        for (const auto line_markers = splitString(marker_literal);
            const auto & line_marker : line_markers)
        {
            defined_line_marker.emplace(line_marker,
                std::pair < uint64_t, std::vector < uint64_t > > (0, { }));
        }
    }
}

void process_equ(const std::string& input, std::map < std::string, std::string > & equ_replacement)
{
    // .equ 'Extended Regular Expression', 'Replacement'
    // this is marked, process is done when the whole block is processed before compile
    if (std::smatch matches; std::regex_search(input, matches, equ_pattern)) {
        auto operand1 = matches[1].str();
        auto operand2 = matches[2].str();
        equ_replacement.emplace(operand1, operand2);
    }
}

void sed_equ(std::string& input, std::map < std::string, std::string > & equ_replacement)
{
    for (const auto & [key, value] : equ_replacement) {
        replace_all(input, key, value);
    }
}

void CodeProcessing(
    std::vector < uint8_t > & compiled_code,
    std::vector < std::string > & file)
{
    uint64_t org;
    defined_line_marker_t defined_line_marker;
    std::map < std::string, std::string > equ_replacement;
    uint64_t line_numer = 0;

    auto getline = [](std::vector < std::string > & file_, std::string & line)->bool
    {
        if (file_.empty()) {
            return false;
        }

        line = file_.front();
        return true;
    };

    auto pop_front = [&file, &line_numer]()
    {
        line_numer++;
        file.erase(file.begin());
    };

    std::string line;
    while (getline(file, line))
    {
        auto tmp = line;
        replace_all(tmp, " ", "");
        if (tmp.empty()) {
            pop_front();
            continue;
        }

        // remove comments
        line = truncateAfterSemicolonOrHash(line);
        // remove '\t'
        replace_all(line, "\t", "    ");

        try {
            // search for each preprocessor pattern
            if (std::regex_match(line, org_pattern)) {
                process_org(line, org);
            } else if (std::regex_match(line, lab_pattern)) {
                process_lab(line, defined_line_marker);
            } else if (std::regex_match(line, equ_pattern)) {
                process_equ(line, equ_replacement);
            } else {
                break;
            }
        } catch (const std::exception & e) {
            throw SysdarftPreProcessorError("Line: " + std::to_string(line_numer)
                + ": Error occurred when processing " + line + ": " + e.what());
        }

        pop_front();
    }

    std::vector < std::vector <uint8_t> > code_for_this_block;
    code_for_this_block.emplace_back(compiled_code);
    std::stringstream code_block;

    while (getline(file, line))
    {
        // remove comments
        line = truncateAfterSemicolonOrHash(line);
        // remove '\t'
        replace_all(line, "\t", "    ");
        // search for each preprocessor pattern
        if (std::regex_match(line, org_pattern)             ||
            std::regex_match(line, lab_pattern)             ||
            std::regex_match(line, equ_pattern))
        {
            throw SysdarftPreProcessorError("Line: " + std::to_string(line_numer)
                + ": PreProcessing indicator found after declaration space: " + line);
        }

        // not a preprocessor
        sed_equ(line, equ_replacement);
        // replace_all(line, ":", ":\n"); We revoked this so that line number can count correctly
        code_block << line << std::endl;
        // we don't use pop_front(); since we want to preserve line number offset
        file.erase(file.begin());
    }

    auto str = code_block.str();
    SysdarftCompile(code_for_this_block, code_block, org, defined_line_marker, line_numer);

    // no additional processing from line marker should be performed after this block
    for (auto marker : defined_line_marker) {
        marker.second.second.clear();
    }

    if (code_for_this_block.size() > 1)
    {
        code_for_this_block.erase(code_for_this_block.begin());
        for (const auto & cl : code_for_this_block)
        {
            for (const auto & c : cl) {
                compiled_code.emplace_back(c);
            }
        }
    }
}

void CodeProcessing(std::vector <uint8_t> & code, std::basic_istream<char>& file)
{
    std::vector < std::string > lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.emplace_back(line);
    }

    CodeProcessing(code, lines);
}
