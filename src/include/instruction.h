#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Width
#define _8bit_prefix  0x08
#define _16bit_prefix 0x16
#define _32bit_prefix 0x32
#define _64bit_prefix 0x64
#define FLOATING_POINT_PREFIX 0xFC

// Prefix
#define REGISTER_PREFIX 0x01
#define CONSTANT_PREFIX 0x02
#define MEMORY_PREFIX   0x03

// Specific Registers
#define R_StackPointer                     0xA0
#define R_StackConfiguration               0xA1
#define R_CodeConfiguration                0xA2
#define R_DataPointer                      0xA3
#define R_DataConfiguration                0xA4
#define R_ExtendedSegmentPointer           0xA5
#define R_ExtendedSegmentConfiguration     0xA6
#define R_SegmentationAccessTable          0xA7
#define R_ControlRegister0                 0xA8

#include <string>
#include <algorithm>
#include <cassert>
#include <any>
#include <cstdint>
#include <vector>
#include <debug.h>
#include <instruction_definition.h>

struct EXPORT parsed_target_t
{
    enum { NOTaValidType, REGISTER, CONSTANT, MEMORY } TargetType;
    std::string RegisterName;
    std::string ConstantExpression;

    struct {
        std::string MemoryAccessRatio;
        std::string MemoryBaseAddress;
        std::string MemoryOffset1;
        std::string MemoryOffset2;
        std::string MemoryWidth;
    } memory;
};

parsed_target_t encode_target(std::vector<uint8_t> & buffer, const std::string& input);
void EXPORT encode_instruction(std::vector<uint8_t> & buffer, const std::string & instruction);
void decode_target(std::vector<std::string> & output, std::vector<uint8_t> & input);
void EXPORT decode_instruction(std::vector< std::string > & output_buffer, std::vector<uint8_t> & input_buffer);

inline std::string & remove_space(std::string & str)
{
    std::erase(str, ' ');
    return str;
}

// Function to convert a string to uppercase
inline std::string & capitalization(std::string& input)
{
    std::ranges::transform(input, input.begin(),
                           [](const unsigned char c) { return std::toupper(c); });
    return input;
}

template < unsigned int LENGTH >
void push(std::vector<uint8_t> & buffer, const void * value)
{
    static_assert(LENGTH % 8 == 0);
    assert(value != nullptr);

    for (unsigned int i = 0; i < LENGTH / 8; i ++) {
        buffer.push_back(static_cast<const uint8_t*>(value)[i]);
    }
}

inline void push8(std::vector<uint8_t> & buffer, const uint8_t value) {
    push<8>(buffer, &value);
}

inline void push16(std::vector<uint8_t> & buffer, const uint16_t value) {
    push<16>(buffer, &value);
}

inline void push32(std::vector<uint8_t> & buffer, const uint32_t value) {
    push<32>(buffer, &value);
}

inline void push64(std::vector<uint8_t> & buffer, const uint64_t value) {
    push<64>(buffer, &value);
}

class TargetExpressionError final : public SysdarftBaseError
{
public:
    explicit TargetExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

#endif // INSTRUCTIONS_H
