#include <debug.h>
#include <instruction.h>
#include <vector>
#include <regex>
#include <iomanip>

std::regex target_pattern(R"(<\s*(?:\*\s*(?:1|2|4|8|16)\s*\([^,]+,[^,]+,[^,]+\)|%(?:R|EXR|HER|FER)[0-7]|\$\s*\(\s*(?:0[xX][A-Fa-f0-9]+|\s|[+\-',*\/^%()xX0-9-])+\s*\))\s*>)");
std::regex instruction_pattern(R"(([A-Z]+))");
std::regex operation_width(R"(.8BIT|.16BIT|.32BIT|.64BIT)");

class InstructionExpressionError final : public SysdarftBaseError {
public:
    explicit InstructionExpressionError(const std::string& message) :
        SysdarftBaseError("Instruction Expression Error: " + message) { }
};

std::vector<std::string> clean_line(const std::string & _input)
{
    std::vector<std::string> ret;
    std::string input = _input;

    // capitalize the whole string
    capitalization(input);

    if (std::smatch matches; std::regex_search(input, matches, instruction_pattern)) {
        auto match = matches[0].str();
        ret.emplace_back(remove_space(match));
    } else {
        throw InstructionExpressionError(input);
    }

    if (std::smatch matches; std::regex_search(input, matches, operation_width)) {
        auto match = matches[0].str();
        ret.emplace_back(remove_space(match));
    }

    // Create iterators to traverse all matches
    const auto matches_begin = std::sregex_iterator(input.begin(), input.end(), target_pattern);
    const auto matches_end = std::sregex_iterator();

    // Iterate over all matches and process them
    for (std::sregex_iterator i = matches_begin; i != matches_end; ++i)
    {
        auto match = i->str();
        ret.emplace_back(remove_space(match));
    }

    return ret;
}

void encode_width_specifier(std::vector<uint8_t> &buffer, const std::string & specifier)
{
    if      (specifier == ".8BIT")  { push8(buffer, _8bit_prefix);  }
    else if (specifier == ".16BIT") { push8(buffer, _16bit_prefix); }
    else if (specifier == ".32BIT") { push8(buffer, _32bit_prefix); }
    else if (specifier == ".64BIT") { push8(buffer, _64bit_prefix); }
}

void encode_instruction(std::vector<uint8_t> & buffer, const std::string & instruction)
{
    const auto cleaned_line = clean_line(instruction);
    if (!instruction_map.contains(cleaned_line[0])) {
        throw InstructionExpressionError(instruction);
    }

    const auto & instruction_name = cleaned_line[0];
    const auto & argument_count = instruction_map.at(cleaned_line[0]).at(ENTRY_ARGUMENT_COUNT);
    const auto & requires_width_specification =
        instruction_map.at(cleaned_line[0]).at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION);

    int operand_index_begin = 1;
    push64(buffer, instruction_map.at(instruction_name).at(ENTRY_OPCODE));
    if (requires_width_specification != 0) {
        encode_width_specifier(buffer, cleaned_line[1]);
        operand_index_begin++;
    }

    for (int i = 0; i  < argument_count; i++) {
        auto tmp = cleaned_line[i + operand_index_begin];

        // remove < >
        tmp.erase(tmp.begin());
        tmp.pop_back();

        // encode
        encode_target(buffer, tmp);
    }
}

void decode_instruction(std::vector< std::string > & output_buffer, std::vector<uint8_t> & input_buffer)
{
    std::stringstream buffer;
    uint64_t instruction = 0;

    try {
        instruction = pop64(input_buffer);
    } catch (...) {
        output_buffer.emplace_back("(bad)");
        return;
    }

    for (const auto &[fst, snd] : instruction_map)
    {
        if (snd.at(ENTRY_OPCODE) == instruction)
        {
            buffer << fst;

            if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
            {

                switch (pop8(input_buffer))
                {
                    case 0x08: buffer << " .8bit";   break;
                    case 0x16: buffer << " .16bit";  break;
                    case 0x32: buffer << " .32bit";  break;
                    case 0x64: buffer << " .64bit";  break;
                    default:   buffer << " .bad";    break;
                }
            }

            for (int i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
            {
                std::vector < std::string > operands;
                try {
                    decode_target(operands, input_buffer);
                } catch (SysdarftBaseError &) {
                    buffer << " (bad)";
                }

                buffer << " <";
                for (const auto & code : operands) {
                    buffer << code;
                }
                buffer << ">";

                if (i == 0) buffer << ",";
            }

            output_buffer.emplace_back(buffer.str());
            return;
        }
    }

    output_buffer.emplace_back("(bad)");
}

int main()
{
    std::vector <std::string> instructions = {
        "nop",
        "add .8bit <%R0> , <$(123/3^2+0xff)>",
        "add .64bit < *1 (%FER0, %fer1, $(123/3^2-0xff))>, <%FER4>"
    };

    std::vector<uint8_t> encode_buffer;
    for (const auto & instruction : instructions) {
        encode_instruction(encode_buffer, instruction);
    }

    int counter = 0;

    for (const auto & code : encode_buffer) {
        std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex
                  << static_cast<int>(code) << " " << std::flush;
        counter++;
        if (counter == 8) {
            std::cout << std::endl;
            counter = 0;
        }
    }

    if (counter != 0) {
        std::cout << std::endl;
    }

    std::vector<std::string> decode_buffer;
    const auto code_size = encode_buffer.size();
    while (!encode_buffer.empty())
    {
        std::stringstream line;
        line << std::setw(32) << std::setfill('0') << std::hex << code_size - encode_buffer.size() << ": ";
        decode_buffer.emplace_back(line.str());
        decode_instruction(decode_buffer, encode_buffer);
    }

    for (auto it = decode_buffer.begin(); it != decode_buffer.end(); it += 2) {
        std::cout << *it << *(it+1) << std::endl;
    }
}

