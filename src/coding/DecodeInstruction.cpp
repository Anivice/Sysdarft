#include <iomanip>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

void decode_instruction(std::vector < std::string > & output, std::vector<uint8_t> & input)
{
    try
    {
        std::stringstream buffer;
        uint64_t instruction = 0;
        instruction = code_buffer_pop64(input);

        for (const auto &[fst, snd] : instruction_map)
        {
            if (snd.at(ENTRY_OPCODE) == instruction)
            {
                buffer << fst;

                if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
                {
                    switch (code_buffer_pop8(input))
                    {
                    case 0x08: buffer << " .8bit ";  break;
                    case 0x16: buffer << " .16bit";  break;
                    case 0x32: buffer << " .32bit";  break;
                    case 0x64: buffer << " .64bit";  break;
                    default:
                        output.emplace_back("(bad)");
                        return;
                    }
                } else if (snd.at(ENTRY_ARGUMENT_COUNT) > 0) {
                    buffer << " ";
                }

                for (uint64_t i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
                {
                    std::vector < std::string > operands;
                    try {
                        decode_target(operands, input);
                    } catch (SysdarftBaseError &) {
                        output.emplace_back("(bad)");
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
            }
        }
    } catch (...) {
        output.emplace_back("(bad)");
        return;
    }
}
