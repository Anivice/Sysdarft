#include <fstream>
#include <vector>
#include <algorithm>
#include <EncodingDecoding.h>

class SysdarftCompileError final : public SysdarftBaseError {
public:
    explicit SysdarftCompileError(const std::string& msg) : SysdarftBaseError("Error during compilation: " + msg) { }
};

void replace_all(
    std::string & input,
    const std::string & target,
    const std::string & replacement)
{
    if (target.empty()) return; // Avoid infinite loop if target is empty

    size_t pos = 0;
    while ((pos = input.find(target, pos)) != std::string::npos) {
        input.replace(pos, target.length(), replacement);
        pos += replacement.length(); // Move past the replacement to avoid infinite loop
    }
}

const std::regex ascii_value_pattern(R"('([^']*)')");
void process_ascii_value(std::string& input)
{
    std::vector<std::pair<std::string, std::string>> replacements;

    // Create iterators to traverse all matches without modifying input during iteration.
    for (auto it = std::sregex_iterator(input.begin(), input.end(), ascii_value_pattern);
         it != std::sregex_iterator(); ++it)
    {
        const std::smatch& match = *it;

        // Extract the entire quoted substring and the inner content separately.
        std::string full_match = match.str();      // e.g. "'A'"
        std::string inner_content = match.str(1);  // e.g. "A"

        // Validate the inner content.
        if (inner_content.size() != 1) {
            throw std::runtime_error("Error encountered while parsing the ASCII expression: " + full_match);
        }

        // Convert the single character to its decimal ASCII value.
        std::string decimal_value = std::to_string(static_cast<int>(inner_content.at(0)));

        // Save the replacement pair: { substring to replace, replacement string }
        replacements.emplace_back(full_match, decimal_value);
    }

    // Perform all replacements on the original string.
    // This separate loop ensures we modify the input after collecting all matches.
    for (const auto& rep : replacements) {
        replace_all(input, rep.first, rep.second);
    }
}

// TODO: Sanity check to prevent line marker and max uint64 value appear at the same line
const std::vector < uint8_t > tmp_address_hex = {
        0x02, 0x64, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
}; // 33 bytes

void replaceSequence(
    std::vector<uint8_t>& original,
    const std::vector<uint8_t>& target,
    const std::vector<uint8_t>& replacement)
{
    if (target.empty()) {
        return;
    }

    // Use std::search to find the first occurrence of target in original

    if (auto it = std::ranges::search(original, target).begin();
        it != original.end())
    {
        // Erase the target sequence
        it = original.erase(it, it + static_cast<long>(target.size()));

        // Insert the replacement sequence
        original.insert(it, replacement.begin(), replacement.end());
    }
}

void encode_constant_from_uint64_t(std::vector < uint8_t > & code, uint64_t val)
{
    // always 10 bytes
    code.push_back(CONSTANT_PREFIX);
    code.push_back(_64bit_prefix);
    for (uint64_t i = 0; i < sizeof(uint64_t); i++) {
        code.push_back(((uint8_t*)&val)[i]);
    }
}

uint64_t code_size_now(const std::vector < std::vector <uint8_t> > & code)
{
    uint64_t size = 0;
    for (const auto & line : code) {
        size += static_cast<uint64_t>(line.size());
    }

    return size;
}

void SysdarftCompile(
    std::vector < std::vector <uint8_t> > & code,
    std::basic_iostream < char > & file,
    const uint64_t org,
    defined_line_marker_t & defined_line_marker)
{
    std::string line;
    while (std::getline(file, line))
    {
        // discard empty lines
        auto tmp = line;
        replace_all(tmp, " ", "");
        if (tmp.empty()) {
            continue;
        }

        std::vector <uint8_t> code_for_this_instruction;
        if (line.find(':') != std::string::npos)
        {
            // found a line marker in this line
            replace_all(line, " ", "");
            replace_all(line, ":", "");
            // calculate code position
            uint64_t off = 0;
            for (const auto & ins_code : code) {
                off += ins_code.size();
            }

            // register current offset
            defined_line_marker[line].first = off + org;
            continue;
        }

        if (line.find('@') != std::string::npos) {
            replace_all(line, "@", std::to_string(code_size_now(code) + org));
        }

        if (line.find("@@") != std::string::npos) {
            replace_all(line, "@@", std::to_string(org));
        }

        if (std::smatch match;
            std::regex_search(line, match, ascii_value_pattern))
        {
            process_ascii_value(line);
        }

        for (const auto & line_marker : defined_line_marker)
        {
            std::smatch matches;
            if (std::regex marker(R"((.*)(<\s*)" + line_marker.first + R"(\s*>)(.*))");
                std::regex_search(line, matches, marker))
            {
                if (matches.size() != 4) {
                    throw SysdarftAssemblerError("Error encountered while parsing line marker: " + line);
                }
                replace_all(line, matches[2], "<$(0xFFFFFFFFFFFFFFFF)>");
                defined_line_marker[line_marker.first].second.emplace_back(code.size()); // This address usage appeared here
                break;
            }
        }

        encode_instruction(code_for_this_instruction, line);
        code.emplace_back(code_for_this_instruction);
    }

    // process line numbers
    for (const auto & line_marker : defined_line_marker)
    {
        std::vector < uint8_t > replacement;
        encode_constant_from_uint64_t(replacement, line_marker.second.first);
        for (const auto & each_instruction : line_marker.second.second)
        {
            replaceSequence(code[each_instruction], tmp_address_hex, replacement);
        }
    }
}
