#include <instruction.h>
#include <cpu.h>

std::string processor:: rlmode_decode_instruction_within_range(const uint64_t start, const uint64_t length)
{
    uint64_t offset = 0;
    uint64_t IP = 0;
    std::vector < std::string > output_buffer;
    while (offset < length)
    {
        try {
            std::stringstream buffer;
            uint64_t instruction = 0;
            instruction = rlmode_pop64(start, offset);

            for (const auto &[fst, snd] : instruction_map)
            {
                if (snd.at(ENTRY_OPCODE) == instruction)
                {
                    buffer  << _YELLOW_
                            << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
                            << start + IP << _REGULAR_ ": "
                            << _RED_ _BOLD_ << fst << _REGULAR_ " ";

                    if (snd.at(ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION) != 0)
                    {
                        switch (rlmode_pop8(start, offset))
                        {
                        case 0x08: buffer << _PURPLE_ << ".8bit " << _REGULAR_;  break;
                        case 0x16: buffer << _PURPLE_ << ".16bit" << _REGULAR_;  break;
                        case 0x32: buffer << _PURPLE_ << ".32bit" << _REGULAR_;  break;
                        case 0x64: buffer << _PURPLE_ << ".64bit" << _REGULAR_;  break;
                        default: output_buffer.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_);
                        }
                    }

                    for (int i = 0 ; i < snd.at(ENTRY_ARGUMENT_COUNT); i++)
                    {
                        std::vector < std::string > operands;
                        try {
                            rlmode_decode_target(operands, start, offset);
                        } catch (SysdarftBaseError &) {
                            output_buffer.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_);
                        }

                        buffer << (i == 0 ? _BLUE_ _BOLD_ : _CYAN_ _BOLD_) << " <";
                        for (const auto & code : operands) {
                            buffer << code;
                        }
                        buffer << ">" _REGULAR_;

                        if (i == 0 && snd.at(ENTRY_ARGUMENT_COUNT) > 1) {
                            buffer << _GREEN_ "," _REGULAR_;
                        }
                    }

                    output_buffer.emplace_back(buffer.str());
                    IP = offset;
                }
            }
        } catch (...) {
            output_buffer.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_);
        }
    }

    std::string ret;
    for (const auto & code : output_buffer) {
        ret += code + "\n";
    }

    return ret;
}
