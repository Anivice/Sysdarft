#include <debug.h>
#include <instruction.h>
#include <vector>
#include <regex>
#include <iomanip>

std::regex target_pattern(R"(<\s*(?:\*\s*(?:1|2|4|8|16)\s*\([^,]+,[^,]+,[^,]+\)|%(?:R|EXR|HER|FER)[0-7]|%(SP|SC|CC|DP|DC|ESP|ESC)|%XMM[0-7]|\$\s*\(\s*(?:0[xX][A-Fa-f0-9]+|\s|[+\-',*\/^%()xX0-9-])+\s*\))\s*>)");
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
        throw InstructionExpressionError("No match for instruction in " + input);
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

uint8_t encode_width_specifier(std::vector<uint8_t> &buffer, const std::string & specifier)
{
    if (specifier == ".8BIT")  { push8(buffer, _8bit_prefix);  return _8bit_prefix;  }
    if (specifier == ".16BIT") { push8(buffer, _16bit_prefix); return _16bit_prefix; }
    if (specifier == ".32BIT") { push8(buffer, _32bit_prefix); return _32bit_prefix; }
    if (specifier == ".64BIT") { push8(buffer, _64bit_prefix); return _64bit_prefix; }

    throw InstructionExpressionError("Unknown specifier " + specifier);
}

void encode_instruction(std::vector<uint8_t> & buffer, const std::string & instruction)
{
    const auto cleaned_line = clean_line(instruction);
    if (!instruction_map.contains(cleaned_line[0])) {
        throw InstructionExpressionError("Illegal instruction " + instruction);
    }

    const auto & instruction_name = cleaned_line[0];
    const auto & argument_count = instruction_map.at(cleaned_line[0]).at(ENTRY_ARGUMENT_COUNT);
    const auto & requires_width_specification =
        instruction_map.at(cleaned_line[0]).at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION);

    int operand_index_begin = 1;
    uint8_t current_ops_width = 0;

    push64(buffer, instruction_map.at(instruction_name).at(ENTRY_OPCODE));
    if (requires_width_specification != 0)
    {
        if (cleaned_line.size() < 2) {
            throw InstructionExpressionError("Width specification required but not found for " + instruction);
        }

        try {
            current_ops_width = encode_width_specifier(buffer, cleaned_line[1]);
        } catch (const InstructionExpressionError & /* Err */) {
            throw InstructionExpressionError("Illegal width specification for " + instruction);
        }

        operand_index_begin++;
    }

    for (int i = 0; i  < argument_count; i++)
    {
        if (cleaned_line.size() - operand_index_begin < argument_count) {
            throw InstructionExpressionError(
                "Expected " + std::to_string(argument_count) +
                        ", but found " + std::to_string(cleaned_line.size() - operand_index_begin) +
                        instruction);
        }

        auto tmp = cleaned_line[i + operand_index_begin];

        // remove < >
        tmp.erase(tmp.begin());
        tmp.pop_back();

        // encode
        parsed_target_t parsed_target;

        try {
            parsed_target = encode_target(buffer, tmp);
        } catch (const TargetExpressionError & Err) {
            throw InstructionExpressionError("Illegal Target operand expression for " + instruction +
                "\n>>>\n" + Err.what() + "<<<\n");
        }

        auto assertion = [&instruction](const bool & condition) {
            if (!condition) {
                throw InstructionExpressionError("Operation width mismatch for " + instruction);
            }
        };

        // width consistency check
        if (current_ops_width != 0)
        {
            if (parsed_target.TargetType == parsed_target_t::REGISTER)
            {
                switch (current_ops_width)
                {
                    case _8bit_prefix:  assertion(parsed_target.RegisterName[1] == 'R'); break;
                    case _16bit_prefix: assertion(parsed_target.RegisterName[1] == 'E'); break;
                    case _32bit_prefix: assertion(parsed_target.RegisterName[1] == 'H'); break;
                    case _64bit_prefix: assertion(
                        parsed_target.RegisterName[1] == 'F'
                        or parsed_target.RegisterName == "%SP"
                        or parsed_target.RegisterName == "%SC"
                        or parsed_target.RegisterName == "%CC"
                        or parsed_target.RegisterName == "%DP"
                        or parsed_target.RegisterName == "%DC"
                        or parsed_target.RegisterName == "%ESP"
                        or parsed_target.RegisterName == "%ESC");
                    break;

                    // it should never reach this:
                    default: throw InstructionExpressionError("Unknown error for " + instruction);
                }
            }
        }
    }
}

void decode_instruction(std::vector< std::string > & output_buffer, std::vector<uint8_t> & input_buffer)
{
    std::stringstream buffer;
    uint64_t instruction = 0;

    if (static_cast<signed>(input_buffer.size()) < 8) {
        input_buffer.clear();
        return;
    }

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
            buffer << fst << "\t";

            if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
            {

                switch (pop8(input_buffer))
                {
                    case 0x08: buffer << " .8bit ";  break;
                    case 0x16: buffer << " .16bit";  break;
                    case 0x32: buffer << " .32bit";  break;
                    case 0x64: buffer << " .64bit";  break;
                    default:   output_buffer.emplace_back("(bad)");    return;
                }
            }

            for (int i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
            {
                std::vector < std::string > operands;
                try {
                    decode_target(operands, input_buffer);
                } catch (SysdarftBaseError &) {
                    output_buffer.emplace_back("(bad)");
                    return;
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
    debug::verbose = true;

    std::vector <std::string> instructions = {
        "nop",
        "add .8bit <%R0> , <$(123/3^2+0xff)>",
        "add .64bit < *1 (%FER0, %fer1, $(123/3^2-0xff))>, <%FER4>",
        "adc .32bit <*4(%FER0, $(0), $(0))>, <$(12)>",
        "sub .8bit <%R0> , <$(78/23+2-0xfc)>",
        "sbb .32bit <%HER0> , <%HER1>",
        "mul .16bit <%ExR0>",
        "imul .64bit <%FER0>",
        "div  .64bit <%FER0>",
        "idiv .64bit <%FER0>",
        "Neg .32bit <*1(%FER0, %FER1, %FER2)>",
        "cmp .32bit <%HER0>, <$(0)>",
        "mov .64bit <%DP>, <*2(%FER2, %SP, %ESP)>",
        "fadd <%XMM0>, <%XMM1>",
    };

    std::vector<uint8_t> encode_buffer;
    for (const auto & instruction : instructions) {
        try {
            encode_instruction(encode_buffer, instruction);
        } catch (...) {
            std::cout << "Error when assembling " << instruction << std::endl;
            throw;
        }
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
        line << std::setw(sizeof(uint64_t)*2) << std::setfill('0') << std::uppercase << std::hex
             << code_size - encode_buffer.size() << ": ";
        decode_buffer.emplace_back(line.str());
        decode_instruction(decode_buffer, encode_buffer);
    }

    for (auto it = decode_buffer.begin(); it != decode_buffer.end();)
    {
        if (it != decode_buffer.end()) {
            std::cout << *it;
            ++it;
        } else {
            break;
        }

        if (it != decode_buffer.end()) {
            std::cout << *it;
            ++it;
        } else {
            break;
        }

        std::cout << std::endl;
    }
}
