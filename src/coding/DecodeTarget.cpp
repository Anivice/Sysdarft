#include <iomanip>
#include <EncodingDecoding.h>
#include <InstructionSet.h>

void decode_constant(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::stringstream ret;
    const auto & prefix = code_buffer_pop<uint8_t>(input);

    if (prefix == _64bit_prefix) {
        const auto num = code_buffer_pop<uint64_t>(input);
        ret << "$(0x" << std::hex << std::uppercase << num << ")";
    } else if (prefix == _float_ptr_prefix) {
        const auto num = code_buffer_pop<uint64_t>(input);
        const double fltptr = *(double*)&num;
        ret << "$(" << std::fixed << std::setprecision(16) << fltptr << ")";
    } else {
        ret << "(bad)";
    }

    output.emplace_back(ret.str());
}

void decode_register(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::stringstream ret;
    const auto register_size = code_buffer_pop<uint8_t>(input);
    const auto register_index = code_buffer_pop<uint8_t>(input);

    std::string prefix = "%";

    switch (register_size)
    {
    case _8bit_prefix:  prefix += "R"; break;
    case _16bit_prefix: prefix += "EXR"; break;
    case _32bit_prefix: prefix += "HER"; break;
    case _64bit_prefix:
        if (register_index <= 15) {
            prefix += "FER";
            break;
        }

        switch (register_index)
        {
        case R_StackBase:       output.emplace_back("%SB"); return;
        case R_StackPointer:    output.emplace_back("%SP"); return;
        case R_CodeBase:        output.emplace_back("%CB"); return;
        case R_DataBase:        output.emplace_back("%DB"); return;
        case R_DataPointer:     output.emplace_back("%DP"); return;
        case R_ExtendedBase:    output.emplace_back("%EB"); return;
        case R_ExtendedPointer: output.emplace_back("%EP"); return;
        default: output.emplace_back("(bad)"); return;
        }

    case _float_ptr_prefix: prefix += "XMM"; break;
    default: output.emplace_back("(bad)"); return;
    }

    ret << prefix << static_cast<int>(register_index);

    output.push_back(ret.str());
}

void decode_memory(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    std::string width, ratio;
    std::vector<std::string> operands;
    std::stringstream ret;

    switch (code_buffer_pop<uint8_t>(input)) {
    case 0x08: width = "&8"; break;
    case 0x16: width = "&16"; break;
    case 0x32: width = "&32"; break;
    case 0x64: width = "&64"; break;
    default: output.emplace_back("(bad)"); return;
    }

    auto decode_each_parameter = [&operands, &input]()->bool
    {
        switch(code_buffer_pop<uint8_t>(input))
        {
        case REGISTER_PREFIX: decode_register(operands, input); break;
        case CONSTANT_PREFIX: decode_constant(operands, input); break;
        default: operands.emplace_back("(bad)"); return false;
        }

        return true;
    };

    if (!decode_each_parameter()) { return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { return; }
    operands.emplace_back(")");

    switch (code_buffer_pop<uint8_t>(input))
    {
    case 0x01: ratio = "*1";  break;
    case 0x02: ratio = "*2";  break;
    case 0x04: ratio = "*4";  break;
    case 0x08: ratio = "*8";  break;
    case 0x16: ratio = "*16"; break;
    default: output.emplace_back("(bad)"); return;
    }

    ret << ratio <<  width;
    for (const auto & operand : operands) {
        ret << operand;
    }

    output.push_back(ret.str());
}

void decode_target(std::vector<std::string> & output, std::vector < uint8_t > & input)
{
    switch (code_buffer_pop<uint8_t>(input))
    {
    case REGISTER_PREFIX: decode_register(output, input); break;
    case CONSTANT_PREFIX: decode_constant(output, input); break;
    case MEMORY_PREFIX: decode_memory(output, input); break;
    default: output.emplace_back("(bad)");
    }
}
