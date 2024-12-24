#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <thread>
#include <cassert>
#include <cstdint>
#include <debug.h>
#include <any>
#include <cctype>
#include <iomanip>
#include <instruction.h>

// Define regex patterns
const std::regex register_pattern(R"(^%(R|EXR|HER|FER)[0-7]|^%(SP|SC|CC|DP|DC|ESP|ESC)|^%XMM[0-7]$)");
const std::regex constant_pattern(R"(^\$\((.*)\)$)");
const std::regex memory_pattern(R"(^\*(1|2|4|8|16)\(([^,]+),([^,]+),([^,]+)\)$)");
std::regex base16_pattern(R"(0x[0-9A-Fa-f]+)");

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

std::string execute_bc(const std::string& input, const int scale = 0)
{
    std::stringstream cmd;
    cmd << "bc <<< \"scale=" << scale << "; " << input << '"';
    const auto result = debug::exec_command("sh", "-c", cmd.str().c_str());
    auto cmd_str = cmd.str();
    if (result.exit_status != 0) {
        throw TargetExpressionError(input + ": " + std::to_string(result.exit_status));
    }

    return result.fd_stdout;
}

void encode_register(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    const char suffix [2] = { input.RegisterName.back(), 0 };
    const auto register_index = static_cast<uint8_t>(strtol(reinterpret_cast<const char *>(&suffix), nullptr, 10));

    push8(buffer, REGISTER_PREFIX);
    switch (input.RegisterName[1])
    {
        case 'R' : /* 8bit Register */  push8(buffer,  _8bit_prefix); push8(buffer, register_index); return;
        case 'E' : /* 16bit Register */ push8(buffer, _16bit_prefix); push8(buffer, register_index); return;
        case 'H' : /* 32bit Register */ push8(buffer, _32bit_prefix); push8(buffer, register_index); return;
        case 'F' : /* 64bit Register */ push8(buffer, _64bit_prefix); push8(buffer, register_index); return;
        case 'X' : /* floating-point Register */ push8(buffer, FLOATING_POINT_PREFIX);
                                                 push8(buffer, register_index); return;
    }

    if (input.RegisterName == "%SP")  { push8(buffer, _64bit_prefix); push8(buffer, StackPointer); }
    else if (input.RegisterName == "%SC")  { push8(buffer, _64bit_prefix); push8(buffer, StackConfiguration); }
    else if (input.RegisterName == "%CC")  { push8(buffer, _64bit_prefix); push8(buffer, CodeConfiguration); }
    else if (input.RegisterName == "%DP")  { push8(buffer, _64bit_prefix); push8(buffer, DataPointer); }
    else if (input.RegisterName == "%DC")  { push8(buffer, _64bit_prefix); push8(buffer, DataConfiguration); }
    else if (input.RegisterName == "%ESP") { push8(buffer, _64bit_prefix); push8(buffer, ExtendedSegmentPointer); }
    else if (input.RegisterName == "%ESC") { push8(buffer, _64bit_prefix); push8(buffer, ExtendedSegmentConfiguration); }
    else {
        throw TargetExpressionError("Unknown register " + input.RegisterName);
    }
}

void encode_constant(std::vector<uint8_t> & buffer, const parsed_target_t & input)
{
    auto tmp = input.ConstantExpression;

    if (tmp.size() <= 2) {
        throw TargetExpressionError(input.ConstantExpression);
    }

    // remove last '('
    tmp.pop_back();
    // remove '$('
    tmp.erase(tmp.begin());
    tmp.erase(tmp.begin());

    // replace base 16 value to base 10 value
    process_base16(tmp);

    push8(buffer, CONSTANT_PREFIX);
    const auto result_from_bc = execute_bc(tmp);
    const __int128_t result_cmp1 = strtoll(result_from_bc.c_str(), nullptr, 10);
    const __int128_t result_cmp2 = strtoull(result_from_bc.c_str(), nullptr, 10);
    __int128_t result;

    if (result_cmp1 == result_cmp2) {
        result = result_cmp1;
    }
    else // two compliments are not equal
    {
        if (tmp.front() == '-') { // signed value
            result = result_cmp1; // use the signed result
        } else { // overflow for signed int, use the unsigned value
            result = result_cmp2;
        }
    }

    if (result < 0) { // signed
        push8(buffer, 0x01);
    } else {
        push8(buffer, 0x00);
    }

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
            if (tmp != "%FER" and param != "%SP" and param != "%DP" and param != "%ESP") {
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
        throw TargetExpressionError(input);
    }

    return parsed;
}

void decode_register(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    std::stringstream ret;
    const auto register_size = pop8(input);
    const auto register_index = pop8(input);

    std::string prefix = "%";

    switch (register_size)
    {
        case _8bit_prefix:  prefix += "R"; break;
        case _16bit_prefix: prefix += "EXR"; break;
        case _32bit_prefix: prefix += "HER"; break;
        case _64bit_prefix:
            if (register_index <= 7)
            {
                prefix += "FER";
                break;
            }

            switch (register_index)
            {
                case StackPointer:          output.push_back("%SP"); return;
                case StackConfiguration:    output.push_back("%SC"); return;
                case CodeConfiguration:     output.push_back("%CC"); return;
                case DataPointer:           output.push_back("%DP"); return;
                case DataConfiguration:     output.push_back("%DC"); return;
                case ExtendedSegmentConfiguration: output.push_back("%ESC"); return;
                case ExtendedSegmentPointer: output.push_back("%ESP"); return;
                default: throw TargetExpressionError("Unknown register");
            }

        case FLOATING_POINT_PREFIX: prefix += "XMM"; break;
    }

    ret << prefix << static_cast<int>(register_index);

    output.push_back(ret.str());
}

void decode_constant(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    std::stringstream ret;
    const auto sign = pop8(input);
    const auto num = std::any_cast<uint64_t>(pop<64>(input));
    const int64_t signed_num = *(int64_t*)(void*)&num;
    ret << "$(";

    if (sign != 0x00) {
        ret << signed_num;
    } else {
        ret << "0x" << std::hex << std::uppercase << num;
    }

    ret << ")";
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

void decode_target(std::vector<std::string> & output, std::vector<uint8_t> & input)
{
    switch (pop8(input))
    {
        case REGISTER_PREFIX: decode_register(output, input); break;
        case CONSTANT_PREFIX: decode_constant(output, input); break;
        case MEMORY_PREFIX: decode_memory(output, input); break;
        default: throw TargetExpressionError("Unrecognized Target prefix");
    }
}

// int main(int argc, char ** argv)
// {
//     debug::verbose = true;
//
//     std::vector < std::string > input {
//         "*1($(1),$(2),$(3))",
//         "*2(%FER0, %FER1, $(234 / 2))",
//         "*4(%FER1, %FER2, $( (2^64 -1 ) - 0xFF + 0x12) ) ",
//         "%R7",
//         "%HER4",
//         "$(-1)"
//     };
//
//     for (const auto & inp : input)
//     {
//         std::vector<uint8_t> encode_buffer;
//         std::vector<std::string> decode_buffer;
//         encode_target(encode_buffer, inp);
//
//         for (const auto & code : encode_buffer) {
//             std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << static_cast<int>(code) << " " << std::flush;
//         }
//         std::cout << std::endl;
//
//         decode_target(decode_buffer, encode_buffer);
//         for (const auto & code : decode_buffer) {
//             std::cout << code << std::flush;
//         }
//         std::cout << std::endl;
//     }
// }
