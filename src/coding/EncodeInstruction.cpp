#include <vector>
#include <regex>
#include <iomanip>
#include <EncodingDecoding.h>
#include <SysdarftDebug.h>
#include <InstructionSet.h>

std::regex target_pattern(R"(<\s*(?:\*\s*(?:1|2|4|8|16)\&(8|16|32|64)\s*\([^,]+,[^,]+,[^,]+\)|%(?:R|EXR|HER)[0-7]|%(FER)([\d]+)|%(SP|DP|ESP|CR0)|%XMM[0-7]|\$\s*\(\s*(?:0[xX][A-Fa-f0-9]+|\s|[+\-',*\/^%()xX0-9-])+\s*\))\s*>)");
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
    if (specifier == ".8BIT")  { code_buffer_push8(buffer, _8bit_prefix);  return _8bit_prefix;  }
    if (specifier == ".16BIT") { code_buffer_push8(buffer, _16bit_prefix); return _16bit_prefix; }
    if (specifier == ".32BIT") { code_buffer_push8(buffer, _32bit_prefix); return _32bit_prefix; }
    if (specifier == ".64BIT") { code_buffer_push8(buffer, _64bit_prefix); return _64bit_prefix; }

    throw InstructionExpressionError("Unknown specifier " + specifier);
}

void SYSDARFT_EXPORT_SYMBOL encode_instruction(std::vector<uint8_t> & buffer, const std::string & instruction)
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

    code_buffer_push64(buffer, instruction_map.at(instruction_name).at(ENTRY_OPCODE));
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

    for (uint64_t i = 0; i  < argument_count; i++)
    {
        if (cleaned_line.size() - operand_index_begin < argument_count) {
            throw InstructionExpressionError(
                "Expected " + std::to_string(argument_count) +
                        ", but found " + std::to_string(cleaned_line.size() - operand_index_begin) +
                        ": " + instruction);
        }

        auto tmp = cleaned_line[i + operand_index_begin];

        // remove < >
        tmp.erase(tmp.begin());
        tmp.pop_back();

        // encode
        parsed_target_t parsed_target;

        try {
            parsed_target = encode_target(buffer, tmp);
        } catch (const SysdarftCodeExpressionError & Err) {
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
                        || parsed_target.RegisterName == "%SP"
                        || parsed_target.RegisterName == "%DP"
                        || parsed_target.RegisterName == "%ESP"
                        || parsed_target.RegisterName == "%CR0");
                    break;

                    // it should never reach this:
                    default: throw InstructionExpressionError("Unknown error for " + instruction);
                }
            }
        }
    }
}
