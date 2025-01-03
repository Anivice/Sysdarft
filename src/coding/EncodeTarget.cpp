#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <thread>
#include <cstdint>
#include <cctype>
#include <iomanip>
#include <charconv>
#include <optional>
#include <system_error>
#include <EncodingDecoding.h>
#include <SysdarftDebug.h>

// Define regex patterns
const std::regex register_pattern(R"(^%(R|EXR|HER)[0-7]|%(FER)([\d]+)|^%(SB|SP|CB|DB|DP|EB|EP)|^%XMM[0-5]$)");
const std::regex constant_pattern(R"(^\$\((.*)\)$)");
const std::regex memory_pattern(R"(^\*(1|2|4|8|16)\&(8|16|32|64)\(([^,]+),([^,]+),([^,]+)\)$)");
const std::regex base16_pattern(R"(0x[0-9A-Fa-f]+)");

bool is_valid_register(const std::string& input) {
    return std::regex_match(input, register_pattern);
}

bool is_valid_constant(const std::string& input) {
    return std::regex_match(input, constant_pattern);
}

bool is_valid_memory(const std::string& input) {
    return std::regex_match(input, memory_pattern);
}

parsed_target_t parse(std::string input)
{
    // remove all spaces in input source
    remove_space(input);
    // capitalize all character
    capitalization(input);

    parsed_target_t result { };

    // if it is a register
    if (is_valid_register(input))
    {
        result.RegisterName = input;
        result.TargetType = parsed_target_t::REGISTER;
        return result;
    }
    else if (is_valid_constant(input))
    {
        result.ConstantExpression = input;
        result.TargetType = parsed_target_t::CONSTANT;
        return result;
    }
    else if (is_valid_memory(input))
    {
        if (std::smatch matches; std::regex_search(input, matches, memory_pattern))
        {
            if (matches.size() != 6) {
                throw SysdarftCodeExpressionError(input);
            }

            result.TargetType = parsed_target_t::MEMORY;
            result.memory.MemoryAccessRatio = matches[1].str();
            result.memory.MemoryWidth       = matches[2].str();
            result.memory.MemoryBaseAddress = matches[3].str();
            result.memory.MemoryOffset1 = matches[4].str();
            result.memory.MemoryOffset2 = matches[5].str();
            return result;
        }
    } else {
        throw SysdarftCodeExpressionError(input);
    }

    return result;
}

void process_base16(std::string & input)
{
    std::vector < std::pair < std::string, uint64_t > > data;
    auto replace_all = [&input](const std::string & target, const std::string & replacement)
    {
        if (target.empty()) return; // Avoid infinite loop if target is empty

        size_t pos = 0;
        while ((pos = input.find(target, pos)) != std::string::npos) {
            input.replace(pos, target.length(), replacement);
            pos += replacement.length(); // Move past the replacement to avoid infinite loop
        }
    };

    // fix the uppercase conversion
    replace_all("0X", "0x");

    // Create iterators to traverse all matches
    const auto matches_begin = std::sregex_iterator(input.begin(), input.end(), base16_pattern);
    const auto matches_end = std::sregex_iterator();

    // Iterate over all matches and process them
    for (std::sregex_iterator i = matches_begin; i != matches_end; ++i)
    {
        const std::smatch& match = *i;
        std::string base16_number_str = match.str();

        // Convert the hexadecimal string to an unsigned 64-bit integer
        uint64_t number = strtoull(base16_number_str.c_str(), nullptr, 16);

        // Store the pair in the data vector
        data.emplace_back(base16_number_str, number);
    }

    for (const auto & [tag, rep] : data) {
        replace_all(tag, std::to_string(rep));
    }
}

std::string execute_bc(const std::string& input)
{
    std::stringstream cmd;
    cmd << "bc <<< \"" << input << '"';
    const auto result = debug::exec_command("bash", "-c", cmd.str().c_str());
    auto cmd_str = cmd.str();
    if (result.exit_status != 0) {
        throw SysdarftCodeExpressionError(input + ": " + std::to_string(result.exit_status));
    }

    return result.fd_stdout;
}

// Function to extract trailing digits and convert to uint32_t
std::optional<uint32_t> extractTrailingNumber(const std::string& input)
{
    // Step 1: Find the position where trailing digits start
    size_t pos = input.size();
    while (pos > 0 && std::isdigit(static_cast<unsigned char>(input[pos - 1]))) {
        --pos;
    }

    // If no digits are found at the end, return std::nullopt
    if (pos == input.size()) {
        return std::nullopt;
    }

    // Step 2: Extract the trailing number substring
    const std::string numberStr = input.substr(pos);

    // Step 3: Convert the substring to uint32_t using std::from_chars
    uint32_t number = 0;

    // Step 4: Handle potential errors
    if (auto [ptr, ec] = std::from_chars(
        numberStr.data(),
        numberStr.data() + numberStr.size(),
        number);
        ec == std::errc())
    {
        return number;
    } else {
        // Conversion failed (e.g., overflow or invalid input)
        return std::nullopt;
    }
}

void encode_register(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    const auto register_index =  extractTrailingNumber(input.RegisterName).has_value() ?
        extractTrailingNumber(input.RegisterName).value() : 0;
    code_buffer_push8(buffer, REGISTER_PREFIX);

    if (input.RegisterName == "%SB") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_StackBase);
        return;
    }

    if (input.RegisterName == "%SP") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_StackPointer);
        return;
    }

    if (input.RegisterName == "%CB") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_CodeBase);
        return;
    }

    if (input.RegisterName == "%DB") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_DataBase);
        return;
    }

    if (input.RegisterName == "%DP") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_DataPointer);
        return;
    }

    if (input.RegisterName == "%EB") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_ExtendedBase);
        return;
    }

    if (input.RegisterName == "%EP") {
        code_buffer_push8(buffer, _64bit_prefix);
        code_buffer_push8(buffer, R_ExtendedPointer);
        return;
    }

    switch (input.RegisterName[1])
    {
    case 'R' : /* 8bit Register */  code_buffer_push8(buffer,  _8bit_prefix); code_buffer_push8(buffer, register_index); return;
    case 'E' : /* 16bit Register */ code_buffer_push8(buffer, _16bit_prefix); code_buffer_push8(buffer, register_index); return;
    case 'H' : /* 32bit Register */ code_buffer_push8(buffer, _32bit_prefix); code_buffer_push8(buffer, register_index); return;
    case 'F' : /* 64bit Register */ code_buffer_push8(buffer, _64bit_prefix); code_buffer_push8(buffer, register_index); return;
    case 'X' : /* floating-point Register */ code_buffer_push8(buffer, _float_ptr_prefix); code_buffer_push8(buffer, register_index); return;
    default: throw SysdarftCodeExpressionError("Unknown register " + input.RegisterName);
    }
}

void encode_constant(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    auto tmp = input.ConstantExpression;

    if (tmp.size() <= 2) {
        throw SysdarftCodeExpressionError(input.ConstantExpression);
    }

    // remove last '('
    tmp.pop_back();
    // remove '$('
    tmp.erase(tmp.begin());
    tmp.erase(tmp.begin());

    // replace base 16 value to base 10 value
    process_base16(tmp);

    code_buffer_push8(buffer, CONSTANT_PREFIX);
    const auto result_from_bc = execute_bc(tmp);

    if (result_from_bc.contains('.')) {
        code_buffer_push8(buffer, _float_ptr_prefix);
        const double result = strtod(result_from_bc.c_str(), nullptr);
        code_buffer_push<64>(buffer, &result);
    } else {
        code_buffer_push8(buffer, _64bit_prefix);
        const __int128_t result = strtoull(result_from_bc.c_str(), nullptr, 10);
        code_buffer_push<64>(buffer, &result);
    }
}

void encode_memory_width_prefix(std::vector<uint8_t> & buffer, const std::string & input)
{
    if (input == "8") { code_buffer_push8(buffer, _8bit_prefix); return;}
    else if (input == "16") { code_buffer_push8(buffer, _16bit_prefix); return;}
    else if (input == "32") { code_buffer_push8(buffer, _32bit_prefix); return;}
    else if (input == "64") { code_buffer_push8(buffer, _64bit_prefix); return;}
    else { throw SysdarftCodeExpressionError(input); }
}

void encode_memory(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    code_buffer_push8(buffer, MEMORY_PREFIX);

    auto encode_each_parameter = [&buffer](const std::string & param)
    {
        if (is_valid_register(param))
        {
            auto tmp = param;
            while (std::isdigit(tmp.back())) {
                tmp.pop_back();
            }

            // Not a 64bit register
            if (tmp != "%FER" && param != "%SP" && param != "%DP" && param != "%ESP") {
                throw SysdarftCodeExpressionError("Not a 64bit Register: " + param);
            }

            encode_register(buffer,
                parsed_target_t {
                    .TargetType = parsed_target_t::REGISTER,
                    .RegisterName = param }
            );
        }
        else if (is_valid_constant(param))
        {
            encode_constant(buffer, parsed_target_t {
                    .TargetType = parsed_target_t::CONSTANT,
                    .ConstantExpression = param }
            );
        } else {
            throw SysdarftCodeExpressionError(param);
        }
    };

    encode_memory_width_prefix(buffer, input.memory.MemoryWidth);
    encode_each_parameter(input.memory.MemoryBaseAddress);
    encode_each_parameter(input.memory.MemoryOffset1);
    encode_each_parameter(input.memory.MemoryOffset2);

    // Ratio. Ratio is a 8bit packed BCD code
    if (input.memory.MemoryAccessRatio == "1") {
        code_buffer_push8(buffer, 0x01);
    } else if (input.memory.MemoryAccessRatio == "2") {
        code_buffer_push8(buffer, 0x02);
    } else if (input.memory.MemoryAccessRatio == "4") {
        code_buffer_push8(buffer, 0x04);
    } else if (input.memory.MemoryAccessRatio == "8") {
        code_buffer_push8(buffer, 0x08);
    } else if (input.memory.MemoryAccessRatio == "16") {
        code_buffer_push8(buffer, 0x16);
    }
}

parsed_target_t encode_target(std::vector<uint8_t> & buffer, const std::string& input)
{
    const auto parsed = parse(input);

    if (parsed.TargetType == parsed_target_t::REGISTER)
    {
        encode_register(buffer, parsed);
    } else if (parsed.TargetType == parsed_target_t::CONSTANT) {
        encode_constant(buffer, parsed);
    } else if (parsed.TargetType == parsed_target_t::MEMORY) {
        encode_memory(buffer, parsed);
    } else {
        throw SysdarftCodeExpressionError(input);
    }

    return parsed;
}
