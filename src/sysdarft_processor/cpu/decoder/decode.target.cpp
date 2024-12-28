#include <instruction.h>
#include <cpu.h>

void processor::rlmode_decode_constant(std::vector<std::string> & output, const uint64_t begin, uint64_t & offset)
{
    std::stringstream ret;
    if (rlmode_pop8(begin, offset) != 0x64) {
        output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_);
        return;
    }

    const auto num = std::any_cast<uint64_t>(rlmode_pop64(begin, offset));
    ret << "$(0x" << std::hex << std::uppercase << num << ")";
    output.push_back(ret.str());
}

void processor::rlmode_decode_register(std::vector<std::string> & output, const uint64_t begin, uint64_t & offset)
{
    std::stringstream ret;
    const auto register_size = rlmode_pop8(begin, offset);
    const auto register_index = rlmode_pop8(begin, offset);

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
        case R_StackPointer:           output.emplace_back("%SP"); return;
        case R_DataPointer:            output.emplace_back("%DP"); return;
        case R_ExtendedSegmentPointer: output.emplace_back("%ESP"); return;
        default: output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return;
        }

    case FLOATING_POINT_PREFIX: prefix += "XMM"; break;
    default: output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return;
    }

    ret << prefix << static_cast<int>(register_index);

    output.push_back(ret.str());
}

void processor::rlmode_decode_memory(std::vector<std::string> & output, const uint64_t begin, uint64_t & offset)
{
    std::string width, ratio;
    std::vector<std::string> operands;
    std::stringstream ret;

    switch (rlmode_pop8(begin, offset)) {
    case 0x08: width = "&8"; break;
    case 0x16: width = "&16"; break;
    case 0x32: width = "&32"; break;
    case 0x64: width = "&64"; break;
    default: output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return;
    }

    auto decode_each_parameter = [&operands, &begin, &offset, this]()->bool
    {
        switch(rlmode_pop8(begin, offset))
        {
        case REGISTER_PREFIX: rlmode_decode_register(operands, begin, offset); break;
        case CONSTANT_PREFIX: rlmode_decode_constant(operands, begin, offset); break;
        default: operands.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return false;
        }

        return true;
    };

    if (!decode_each_parameter()) { output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return; }
    operands.emplace_back(", ");
    if (!decode_each_parameter()) { output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return; }
    operands.emplace_back(")");

    switch (auto _ratio = rlmode_pop8(begin, offset))
    {
    case 0x01: ratio = "*1";  break;
    case 0x02: ratio = "*2";  break;
    case 0x04: ratio = "*4";  break;
    case 0x08: ratio = "*8";  break;
    case 0x16: ratio = "*16"; break;
    default: output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_); return;
    }

    ret << ratio <<  width;
    for (const auto & operand : operands) {
        ret << operand;
    }

    output.push_back(ret.str());
}

void processor::rlmode_decode_target(std::vector<std::string> & output, const uint64_t begin, uint64_t & offset)
{
    switch (rlmode_pop8(begin, offset))
    {
    case REGISTER_PREFIX: rlmode_decode_register(output, begin, offset); break;
    case CONSTANT_PREFIX: rlmode_decode_constant(output, begin, offset); break;
    case MEMORY_PREFIX: rlmode_decode_memory(output, begin, offset); break;
    default: output.emplace_back(_RED_ _BOLD_ "(bad)" _REGULAR_);
    }
}
