#include <fstream>
#include <vector>
#include <algorithm>
#include <EncodingDecoding.h>

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

bool process_resvb(std::string& input, std::vector <uint8_t> & code)
{
    const std::regex resvb_pattern(R"(\s*\.resvb\s+<(.*)>\s*)", std::regex_constants::icase); // .org 0x123
    if (std::smatch match;
        std::regex_search(input, match, resvb_pattern))
    {
        auto expression = match[1].str();
        process_base16(expression);
        const auto processed_expression = execute_bc(expression);
        const auto count = std::strtoll(processed_expression.c_str(), nullptr, 10);
        for (long long int i = 0; i < count; i++) {
            code.push_back(0x00);
        }
        return true;
    }

    return false;
}

std::string unescapeString(const std::string& input)
{
    std::string result;
    result.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            // Check the character following the backslash
            char next = input[i + 1];
            switch (next) {
            case 'n': result += '\n'; break;  // Newline
            case 't': result += '\t'; break;  // Tab
            case 'r': result += '\r'; break;  // Carriage return
            case '\\': result += '\\'; break; // Literal backslash
            case '\'': result += '\''; break; // Single quote
            case '\"': result += '\"'; break; // Double quote
                // Add more escape sequences if needed
            default:
                // If not a recognized escape, keep the backslash and the character as-is
                    result += '\\';
                result += next;
                break;
            }
            ++i;  // Skip the next character since it's part of the escape sequence
        } else {
            // Regular character, just append it
            result += input[i];
        }
    }

    return result;
}

bool process_string(std::string& input, std::vector <uint8_t> & code)
{
    const std::regex string_pattern(R"(\s*\.string\s+<\s*\"(.*)\"\s*>\s*)", std::regex_constants::icase);
    if (std::smatch match;
        std::regex_search(input, match, string_pattern))
    {
        // the captured group can be wrong, we have to rely on interation

        // unescape
        input = unescapeString(input);

        // find out fist and last appearance of '"'
        const auto first_of = input.find_first_of('"');
        const auto last_of = input.find_last_of('"');

        // sanity check
        if (first_of == std::string::npos || last_of == std::string::npos || first_of >= last_of) {
            throw SysdarftAssemblerError("Error encountered while parsing string: " + input);
        }

        // process
        const ssize_t length = last_of - first_of - 1;
        if (length < 0) {
            throw SysdarftAssemblerError("Error encountered while parsing string: " + input);
        }
        for (const std::string unescaped_substring = input.substr(first_of + 1, length);
            const auto & c : unescaped_substring)
        {
            code.push_back(c);
        }

        return true;
    }

    return false;
}

struct data_expression_identifier
{
    uint64_t data_appearance;
    uint64_t data_byte_count;
    std::string data_string;
};

bool process_data(const std::string& input, std::vector < data_expression_identifier > & data_processors)
{
    const std::regex data_pattern(R"(\s*\.(8|16|32|64)bit_data\s+<(.*)>\s*)", std::regex_constants::icase);
    if (std::smatch match; std::regex_search(input, match, data_pattern))
    {
        data_expression_identifier identifier { };
        identifier.data_byte_count = std::strtol(match[1].str().c_str(), nullptr, 10) / 8;
        identifier.data_string = match[2].str();
        data_processors.push_back(identifier);
        return true;
    }

    return false;
}

void SysdarftCompile(
    std::vector < std::vector <uint8_t> > & code,
    std::basic_iostream < char > & file,
    const uint64_t org,
    defined_line_marker_t & defined_line_marker,
    uint64_t line_number)
{
    std::vector < data_expression_identifier > data_processors;
    const std::regex line_mark_pattern(R"(\s*([A-Za-z_]\w*)(?=)\s*:\s*)");
    std::string line;
    while (std::getline(file, line))
    {
        line_number++;

        try {
            // discard empty lines
            auto tmp = line;
            replace_all(tmp, " ", "");
            if (tmp.empty()) {
                continue;
            }

            std::vector <uint8_t> code_for_this_instruction;
            if (std::smatch match; std::regex_match(tmp, match, line_mark_pattern))
            {
                // found a line marker in this line
                auto marker = match[1].str();

                // register current offset
                defined_line_marker[marker].first = code_size_now(code) + org;
                continue;
            }

            // preprocessor .string expression
            if (process_string(line, code_for_this_instruction)) {
                code.emplace_back(code_for_this_instruction);
                continue; // preprocessor that does not need to be compiled
            }

            // preprocessor @@ (org)
            if (line.find("@@") != std::string::npos) {
                replace_all(line, "@@", std::to_string(org));
            }

            // preprocessor @ (current offset)
            if (line.find('@') != std::string::npos) {
                replace_all(line, "@", std::to_string(code_size_now(code) + org));
            }

            // preprocessor .resvb expression
            if (process_resvb(line, code_for_this_instruction)) {
                code.emplace_back(code_for_this_instruction);
                continue; // preprocessor that does not need to be compiled
            }

            // preprocessor 'ASCII'
            process_ascii_value(line);

            // process data
            if (process_data(line, data_processors))
            {
                code.emplace_back(data_processors.back().data_byte_count);
                data_processors.back().data_appearance = code.size() - 1;
                continue; // this preprocessor is the most complicated.
                // it needs to handle @ and @@ and all line markers, turn them into actual offsets,
                // then calculate the processed expression using bc
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
        } catch (std::exception & e) {
            throw SysdarftAssemblerError(std::string("Line: ") + std::to_string(line_number)
                + ": Error occurred when compiling: " + std::string(e.what()));
        }
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

    // process data
    for (const auto & [ data_appearance, data_byte_count, data_string ]: data_processors)
    {
        auto expression = data_string;
        // handle all line markers, turn them into actual offsets,
        for (const auto &[fst, snd] : defined_line_marker) {
            replace_all(expression, fst,
                std::to_string(snd.first));
        }

        // then calculate the processed expression using bc
        process_base16(expression);
        auto processed_expression = execute_bc(expression);

        // actual number, signed
        auto data = strtoll(processed_expression.c_str(), nullptr, 10);
        uint64_t compliment = 0xFFFFFFFFFFFFFFFF;
        compliment = compliment >> (64 - (data_byte_count * 8));
        uint64_t raw_data = *(uint64_t*)&data;
        raw_data = raw_data & compliment;

        // emplace data
        std::vector < uint8_t > data_sequence;
        for (uint64_t i = 0; i < data_byte_count; i++) {
            data_sequence.emplace_back(((uint8_t*)&raw_data)[i]);
        }

        code[data_appearance] = data_sequence;
    }
}
