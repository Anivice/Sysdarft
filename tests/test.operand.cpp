#include <SysdarftCPUDecoder.h>
#include <EncodingDecoding.h>

class CodeBase : public DecoderDataAccess {
public:

    CodeBase()
    {
        std::vector<uint8_t> buffer;
        encode_instruction(buffer, "mov .64bit <*1&64($(0), $(0), $(0))>, <%FER6>");

        uint64_t off = BIOS_START;
        for (const auto & code : buffer) {
            write_memory(off++, (char*)&code, 1);
        }

        store<InstructionPointerType>(load<InstructionPointerType>() + 2);
        OperandType operand(*this);
        OperandType operand2(*this);

        operand.set_val(128);
        operand2.set_val(255);
        const uint64_t a = operand.get_val();
        const uint64_t b = operand2.get_val();
        uint64_t c = a + b;
    }
};

int main()
{
    debug::verbose = true;
    CodeBase base;
}

