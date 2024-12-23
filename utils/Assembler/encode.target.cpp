#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <thread>
#include <cassert>
#include <cstdint>
#include <debug.h>
#include <algorithm>
#include <any>
#include <cctype>
#include <iomanip>
#include <utility>

constexpr uint8_t REGISTER_PREFIX= 0x01;
constexpr uint8_t CONSTANT_PREFIX= 0x02;
constexpr uint8_t MEMORY_PREFIX= 0x03;

// Define regex patterns
const std::regex register_pattern(R"(^%(R[0-7]|EXR[0-7]|HER[0-7]|FER[0-7])$)");
const std::regex constant_pattern(R"(^\$([8]|[1][6]|[3][2]|[6][4])\((.*)\)$)");
const std::regex memory_pattern(R"(^\*([1]|[2]|[4]|[8]|[1][6])\((.*),(.*),(.*)\)$)");

struct parsed_target_t
{
    enum { NOTaValidType, REGISTER, CONSTANT, MEMORY } TargetType;
    std::string RegisterName;
    std::string ConstantExpression;

    struct {
        std::string MemoryAccessRatio;
        std::string MemoryBaseAddress;
        std::string MemoryOffset1;
        std::string MemoryOffset2;
    } memory;
};

inline std::string & remove_space(std::string & str)
{
    std::erase(str, ' ');
    return str;
}

template < unsigned int LENGTH >
void push(std::vector<uint8_t> & buffer, const void * value)
{
    static_assert(LENGTH % 8 == 0);
    assert(value != nullptr);

    for (unsigned int i = 0; i < LENGTH / 8; i ++) {
        buffer.push_back(static_cast<const uint8_t*>(value)[i]);
    }
}

inline void push8(std::vector<uint8_t> & buffer, const uint8_t value)
{
    push<8>(buffer, &value);
}

bool is_valid_register(const std::string& input) {
    return std::regex_match(input, register_pattern);
}

bool is_valid_constant(const std::string& input) {
    return std::regex_match(input, constant_pattern);
}

bool is_valid_memory(const std::string& input) {
    return std::regex_match(input, memory_pattern);
}

// Function to convert a string to uppercase
inline std::string & capitalization(std::string& input)
{
    std::ranges::transform(input, input.begin(),
                           [](const unsigned char c) { return std::toupper(c); });
    return input;
}

class TargetExpressionError final : public SysdarftBaseError
{
    public:
    explicit TargetExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

parsed_target_t parse(std::string input)
{
    // remove all spaces in input source
    remove_space(input);
    // capitalize all character
    capitalization(input);

    parsed_target_t result { };

    // if it is a register
    if (is_valid_register(input)) {
        result.RegisterName = input.c_str() + 1; // remove '%'
        result.TargetType = parsed_target_t::REGISTER;
        return result;
    } else if (is_valid_constant(input)) {
        result.ConstantExpression = input;
        result.TargetType = parsed_target_t::CONSTANT;
        return result;
    } else if (is_valid_memory(input)) {
        if (std::smatch matches; std::regex_search(input, matches, memory_pattern))
        {
            if (matches.size() != 5) {
                throw TargetExpressionError(input);
            }

            result.TargetType = parsed_target_t::MEMORY;
            result.memory.MemoryAccessRatio = matches[1].str();
            result.memory.MemoryBaseAddress = matches[2].str();
            result.memory.MemoryOffset1 = matches[3].str();
            result.memory.MemoryOffset2 = matches[4].str();
            return result;
        }
    } else {
        throw TargetExpressionError(input);
    }

    return result;
}

std::string execute_bc(const std::string& input, const int scale = 0)
{
    std::stringstream cmd;
    cmd << "bc <<< \"scale=" << scale << "; " << input << '"';
    const auto result = debug::exec_command("sh", "-c", cmd.str().c_str());

    if (result.exit_status != 0) {
        throw TargetExpressionError(input + ": " + std::to_string(result.exit_status));
    }

    return std::to_string(strtoll(result.fd_stdout.c_str(), nullptr, 10));
}

void encode_register(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    const char suffix [2] = { input.RegisterName.back(), 0 };
    const auto register_index = static_cast<uint8_t>(strtol(reinterpret_cast<const char *>(&suffix), nullptr, 10));

    push8(buffer, REGISTER_PREFIX);
    switch (input.RegisterName[1])
    {
        case 'R' : /* 8bit Register */ push8(buffer, 0x08); break;
        case 'E' : /* 16bit Register */ push8(buffer, 0x16); break;
        case 'H' : /* 32bit Register */ push8(buffer, 0x32); break;
        case 'F' : /* 64bit Register */ push8(buffer, 0x64); break;
        default: throw TargetExpressionError("Unrecognized register name");
    }
    push8(buffer, register_index);
}

/**
 * Splits the input string at the first occurrence of '('.
 *
 * @param input The string to split.
 * @return A pair of strings:
 *         - first: Substring before '('
 *         - second: Substring after '('
 *         If '(' is not found, second will be an empty string.
 */
std::pair<std::string, std::string> split_at_first_parenthesis(const std::string& input)
{
    // Find the position of the first '('
    size_t pos = input.find('(');

    // Check if '(' was found
    if (pos == std::string::npos) {
        // '(' not found; return the entire string as the first part and empty second part
        return {input, ""};
    }

    // Extract the substring before '('
    std::string before = input.substr(0, pos);

    // Extract the substring after '('
    // Adding 1 to pos to exclude '(' itself
    std::string after = input.substr(pos + 1);

    return {before, after};
}

void encode_constant(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    auto tmp = input.ConstantExpression;

    if (tmp.size() <= 2) {
        throw TargetExpressionError(input.ConstantExpression);
    }

    // remove last '('
    tmp.pop_back();

    // split
    auto [fst, snd] = split_at_first_parenthesis(tmp);
    fst.erase(fst.begin());

    push8(buffer, CONSTANT_PREFIX);
    switch (strtol(fst.c_str(), nullptr, 10))
    {
        case 8: buffer.push_back(0x08); break;
        case 16: buffer.push_back(0x16); break;
        case 32: buffer.push_back(0x32); break;
        case 64: buffer.push_back(0x64); break;
        default: throw TargetExpressionError(input.ConstantExpression);
    }

    const auto result_from_bc = execute_bc(snd);
    const uint64_t result = strtoll(result_from_bc.c_str(), nullptr, 10);
    push<64>(buffer, &result);
}

void encode_memory(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    push8(buffer, MEMORY_PREFIX);

    // Ratio. Ratio is a 8bit packed BCD code
    if (input.memory.MemoryAccessRatio == "1") {
        push8(buffer, 0x01);
    } else if (input.memory.MemoryAccessRatio == "2") {
        push8(buffer, 0x02);
    } else if (input.memory.MemoryAccessRatio == "4") {
        push8(buffer, 0x04);
    } else if (input.memory.MemoryAccessRatio == "8") {
        push8(buffer, 0x08);
    } else if (input.memory.MemoryAccessRatio == "16") {
        push8(buffer, 0x16);
    }

    auto encode_each_parameter = [&buffer](const std::string & param)
    {
        if (is_valid_register(param))
        {
            auto tmp = param;
            tmp.pop_back();

            // Not a 64bit register
            if (tmp != "%FER") {
                throw TargetExpressionError("Not a 64bit Register: " + param);
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
            throw TargetExpressionError(param);
        }
    };

    encode_each_parameter(input.memory.MemoryBaseAddress);
    encode_each_parameter(input.memory.MemoryOffset1);
    encode_each_parameter(input.memory.MemoryOffset2);
}

void encode_target(std::vector<uint8_t> & buffer, const std::string& input)
{

    if (const auto parsed = parse(input);
        parsed.TargetType == parsed_target_t::REGISTER)
    {
        encode_register(buffer, parsed);
    } else if (parsed.TargetType == parsed_target_t::CONSTANT) {
        encode_constant(buffer, parsed);
    } else if (parsed.TargetType == parsed_target_t::MEMORY) {
        encode_memory(buffer, parsed);
    } else {
        throw TargetExpressionError(input);
    }
}

template < unsigned int LENGTH >
std::any pop(std::vector<uint8_t> & input)
{
    static_assert(LENGTH % 8 == 0);

    __uint128_t result = 0;
    auto* buffer = (uint8_t*)&result;

    for (unsigned int i = 0; i < LENGTH / 8; i++) {
        buffer[i] = input[0];
        input.erase(input.begin());
    }

    switch (LENGTH / 8)
    {
        case 1: /* 8bit */  return static_cast<uint8_t> (result & 0xFF);
        case 2: /* 16bit */ return static_cast<uint16_t>(result & 0xFFFF);
        case 4: /* 32bit */ return static_cast<uint32_t>(result & 0xFFFFFFFF);
        case 8: /* 64bit */ return static_cast<uint64_t>(result & 0xFFFFFFFFFFFFFFFF);
        default: throw TargetExpressionError("Unrecognized length");
    }
}

inline uint8_t pop8(std::vector<uint8_t> & input) {
    return std::any_cast<uint8_t>(pop<8>(input));
}

void decode_register(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    std::stringstream ret;
    const auto register_size = pop8(input);
    const auto register_index = pop8(input);

    std::string prefix = "%";

    switch (register_size)
    {
        case 0x08: prefix += "R"; break;
        case 0x16: prefix += "EXR"; break;
        case 0x32: prefix += "HER"; break;
        case 0x64: prefix += "FER"; break;
        default: throw TargetExpressionError("Unrecognized register size");
    }

    ret << prefix << static_cast<int>(register_index);

    output.push_back(ret.str());
}

void decode_constant(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    std::stringstream ret;
    switch (pop8(input))
    {
        case 0x08:  ret << "$8("  << static_cast<int>(std::any_cast<uint8_t>(pop<8>(input))) << ")"; break;
        case 0x16: ret << "$16(" << std::any_cast<uint16_t>(pop<16>(input)) << ")"; break;
        case 0x32: ret << "$32(" << std::any_cast<uint32_t>(pop<32>(input)) << ")"; break;
        case 0x64: ret << "$64(" << std::any_cast<uint64_t>(pop<64>(input)) << ")"; break;
        default: throw TargetExpressionError("Unrecognized data width");
    }

    output.push_back(ret.str());
}

void decode_memory(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    std::stringstream ret;
    switch (pop8(input))
    {
        case 0x01: ret << "*1(";  break;
        case 0x02: ret << "*2(";  break;
        case 0x04: ret << "*4(";  break;
        case 0x08: ret << "*8(";  break;
        case 0x16: ret << "*16("; break;
        default: throw TargetExpressionError("Unrecognized memory ratio");
    }

    output.push_back(ret.str());

    auto decode_each_parameter = [&output, &input]()
    {
        switch(pop8(input))
        {
        case REGISTER_PREFIX: decode_register(output, input); break;
        case CONSTANT_PREFIX: decode_constant(output, input); break;
        default: throw TargetExpressionError("Unrecognized parameter prefix");
        }
    };

    decode_each_parameter();
    output.emplace_back(", ");
    decode_each_parameter();
    output.emplace_back(", ");
    decode_each_parameter();
    output.emplace_back(")");
}

void decode(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    switch (pop8(input))
    {
        case REGISTER_PREFIX: decode_register(output, input); break;
        case CONSTANT_PREFIX: decode_constant(output, input); break;
        case MEMORY_PREFIX: decode_memory(output, input); break;
        default: throw TargetExpressionError("Unrecognized Target prefix");
    }
}

int main(int argc, char ** argv)
{
    debug::verbose = true;

    std::vector < std::string > input {
        "*1($64(1),$64(2),$64(3))",
        "*2(%FER0, %FER1, $64(234 / 2))",
        "*4(%FER1, %FER2, $8(2^39))",
    };

    for (const auto & inp : input)
    {
        std::vector<uint8_t> encode_buffer;
        std::vector<std::string> decode_buffer;
        encode_target(encode_buffer, inp);

        for (const auto & code : encode_buffer) {
            std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << static_cast<int>(code) << " " << std::flush;
        }
        std::cout << std::endl;

        decode(decode_buffer, encode_buffer);
        for (const auto & code : decode_buffer) {
            std::cout << code << std::flush;
        }
        std::cout << std::endl;
    }
}
