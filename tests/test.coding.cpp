#include <iomanip>
#include <EncodingDecoding.h>

int main()
{
    debug::verbose = true;
    std::vector<uint8_t> buffer;
    std::vector<std::string> lines;

    encode_instruction(buffer, "add .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "add .64bit <%FER14>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "add .8bit <%R2>, <$(0xFF)>");
    encode_instruction(buffer, "add .8bit <%R3>, <$(0xA0)>");
    encode_instruction(buffer, "add .8bit <%R0>, <$(0x02)>");
    encode_instruction(buffer, "add .8bit <%R1>, <$(0x30)>");
    encode_instruction(buffer, "add .8bit <%R0>, <%R2>");
    encode_instruction(buffer, "adc .8bit <%R1>, <%R3>");
    encode_instruction(buffer, "sub .16bit <%EXR0>, <$(0xFFFF)>");
    encode_instruction(buffer, "mov .16bit <%EXR0>, <$(-32)>");
    encode_instruction(buffer, "imul .16bit <$(-2)>");
    encode_instruction(buffer, "mov .32bit <%HER0>, <$(65536)>");
    encode_instruction(buffer, "mov .32bit <%HER2>, <$(0x02)>");
    encode_instruction(buffer, "mul .32bit <%HER2>");
    encode_instruction(buffer, "mov .64bit <%FER0>, <$(-65536)>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <$(-2)>");
    encode_instruction(buffer, "idiv .64bit <%FER1>");
    encode_instruction(buffer, "div .64bit <$(3)>");
    encode_instruction(buffer, "neg .64bit <%FER0>");
    encode_instruction(buffer, "cmp .16bit <%EXR0>, <%EXR1>");
    encode_instruction(buffer, "nop");
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(6))>, <$(0xFFF)>");
    encode_instruction(buffer, "mov .64bit <%FER0>, <*2&64($(255), %FER14, $(6))>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "xchg .64bit <%FER0>, <%FER1>");
    encode_instruction(buffer, "mov .64bit <%SP>, <$(0xFFFF)>");
    encode_instruction(buffer, "push .64bit <%FER0>");
    encode_instruction(buffer, "pop .64bit <%FER2>");
    encode_instruction(buffer, "pushall");
    encode_instruction(buffer, "div .64bit <%FER1>");
    encode_instruction(buffer, "popall");
    encode_instruction(buffer, "enter .64bit <$(0xFF)>");
    encode_instruction(buffer, "leave");
    encode_instruction(buffer, "mov .64bit <%FER0>, <$(0x00)>");
    encode_instruction(buffer, "mov .64bit <%FER1>, <$(0xC1800)>");
    encode_instruction(buffer, "mov .64bit <%FER2>, <$(0xFFF)>");
    encode_instruction(buffer, "movs");
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(4))>, <$(114514)>");
    encode_instruction(buffer, "mov .64bit <*2&64($(255), %FER14, $(6))>, <$(0xFFF)>");
    encode_instruction(buffer, "and .64bit <*2&64($(255), %FER14, $(4))>, <*2&64($(255), %FER14, $(6))>");
    encode_instruction(buffer, "mov .64bit <%FER0>, <*2&64($(255), %FER14, $(4))>");
    encode_instruction(buffer, "or .32bit <%HER1>, <%HER0>");
    encode_instruction(buffer, "xor .64bit <%FER0>, <%FER0>");
    encode_instruction(buffer, "mov .8bit <%R0>, <$(0x34)>");
    encode_instruction(buffer, "not .64bit <%FER0>");
    encode_instruction(buffer, "shl .8bit <%R0>, <$(4)>");
    encode_instruction(buffer, "shr .8bit <%R0>, <$(6)>");
    encode_instruction(buffer, "mov .8bit <%R0>, <$(0xF4)>");
    encode_instruction(buffer, "rol .8bit <%R0>, <$(2)>");
    encode_instruction(buffer, "ror .8bit <%R0>, <$(1)>");
    encode_instruction(buffer, "mov .8bit <%R0>, <$(0x8F)>");
    encode_instruction(buffer, "rcl .8bit <%R0>, <$(1)>");
    encode_instruction(buffer, "rcr .8bit <%R0>, <$(1)>");
    encode_instruction(buffer, "mov .64bit <%SB>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%SP>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%CB>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%DB>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%DP>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%EB>, <$(0xFF)>");
    encode_instruction(buffer, "mov .64bit <%EP>, <$(0xFF)>");

    uint64_t IP = 0;
    const uint64_t Space = buffer.size();
    while (!buffer.empty())
    {
        std::stringstream line_num;
        line_num
            << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
            << IP;
        lines.emplace_back(line_num.str());
        decode_instruction(lines, buffer);
        IP = Space - buffer.size();
    }

    for (auto it = lines.begin(); it != lines.end(); it += 2) {
        std::cout << *it << ": " << *(it + 1) << std::endl;
    }
}
