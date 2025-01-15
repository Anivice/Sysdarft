#include <iomanip>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

void decode_instruction(std::vector < std::string > & output, std::vector<uint8_t> & input)
{
    try
    {
        std::stringstream buffer;
        const auto instruction = code_buffer_pop8(input);

        for (const auto &[fst, snd] : instruction_map)
        {
            if (snd.at(ENTRY_OPCODE) == instruction)
            {
                buffer << fst;

                uint8_t op_width = 0;
                if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
                {
                    switch (op_width = code_buffer_pop8(input))
                    {
                    case _8bit_prefix:  buffer << " .8bit "; break;
                    case _16bit_prefix: buffer << " .16bit";  break;
                    case _32bit_prefix: buffer << " .32bit";  break;
                    case _64bit_prefix: buffer << " .64bit";  break;
                    default:
                        output.emplace_back(bad_nbit(instruction));
                        output.emplace_back(bad_nbit(op_width));
                        return;
                    }
                }

                for (uint64_t i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
                {
                    std::vector < std::string > operands;
                    try {
                        decode_target(operands, input);
                    } catch (SysdarftBaseError &) {
                        output.emplace_back(bad_nbit(instruction));
                        if (op_width) {
                            output.emplace_back(bad_nbit(op_width));
                        }
                        output.insert(output.end(), operands.begin(), operands.end());
                        return;
                    }

                    buffer << " <";
                    for (const auto & code : operands) {
                        buffer << code;
                    }
                    buffer << ">";

                    if (i == 0 && snd.at(ENTRY_ARGUMENT_COUNT) > 1) {
                        buffer << ",";
                    }
                }

                output.emplace_back(buffer.str());
                return;
            }
        }

        output.emplace_back(bad_nbit(instruction));
    } catch (...) {
        // output.emplace_back("(bad)");
        return;
    }
}
