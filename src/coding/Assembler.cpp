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

const std::regex ascii_value_pattern(R"(\'(.)\')");
void process_ascii_value(std::string& input)
{
    // ... 'A'  ; some instruction, replace 'A' to decimal
    // Create iterators to traverse all matches
    const auto matches_begin = std::sregex_iterator(input.begin(), input.end(), ascii_value_pattern);
    const auto matches_end = std::sregex_iterator();
    std::vector < std::string > result;

    // Iterate over all matches and process them
    for (std::sregex_iterator i = matches_begin; i != matches_end; ++i)
    {
        auto match = i->str();
        result.emplace_back(remove_space(match));
    }

    for (const auto & ascii : result)
    {
        std::string processed = ascii;
        replace_all(processed, "'", "");
        if (processed.size() != 1) {
            throw SysdarftCompileError("Error encountered while parsing the ASCII expression: " + ascii);
        }

        replace_all(input, ascii, std::to_string(static_cast<int>(processed.at(0))));
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
    auto inject_instruction_with_empty_address = [&](std::string & instruction, const std::string & marker)->bool
    {
        if (const auto pos = instruction.find(marker); pos != std::string::npos)
        {
            // way beyond address limit
            replace_all(instruction, marker, "<$(0xFFFFFFFFFFFFFFFF)>");
            return true;
        }

        return false;
    };

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

        if (std::regex_match(line, ascii_value_pattern)) {
            process_ascii_value(line);
        }

        for (const auto & line_marker : defined_line_marker)
        {
            if (line.find(line_marker.first) != std::string::npos)
            {
                if (inject_instruction_with_empty_address(line, line_marker.first)) {
                    defined_line_marker[line_marker.first].second.emplace_back(code.size()); // This address usage appeared here
                }

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
