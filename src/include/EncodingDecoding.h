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
#include <cstdint>
#include <vector>
#include <SysdarftDebug.h>

class SysdarftCodeExpressionError final : public SysdarftBaseError
{
public:
    explicit SysdarftCodeExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

class SysdarftInvalidArgument final : public SysdarftBaseError
{
public:
    explicit SysdarftInvalidArgument(const std::string & message) :
        SysdarftBaseError("Invalid argument provided: " + message) { }
};

class CodeBufferEmptiedWhenPop final : public SysdarftBaseError {
public:
    explicit CodeBufferEmptiedWhenPop(const std::string & msg) :
        SysdarftBaseError("Code buffer emptied before pop finished: " + msg) { }
};

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
void code_buffer_push(std::vector<uint8_t> & buffer, const void * value)
{
    static_assert(LENGTH % 8 == 0);
    if (value == nullptr) {
        throw SysdarftInvalidArgument("Pointer to value is NULL");
    }

    for (unsigned int i = 0; i < LENGTH / 8; i ++) {
        buffer.push_back(((uint8_t*)(value))[i]);
    }
}

inline void code_buffer_push8(std::vector<uint8_t> & buffer, const uint8_t value) {
    code_buffer_push<8>(buffer, &value);
}

inline void code_buffer_push16(std::vector<uint8_t> & buffer, const uint16_t value) {
    code_buffer_push<16>(buffer, &value);
}

inline void code_buffer_push32(std::vector<uint8_t> & buffer, const uint32_t value) {
    code_buffer_push<32>(buffer, &value);
}

inline void code_buffer_push64(std::vector<uint8_t> & buffer, const uint64_t value) {
    code_buffer_push<64>(buffer, &value);
}

template < typename DataType >
DataType code_buffer_pop(std::vector < uint8_t > & input)
{
    DataType Return;
    for (uint64_t i = 0; i < sizeof(DataType); i++)
    {
        if (input.empty()) {
            throw CodeBufferEmptiedWhenPop("No data left before pop finished!");
        }

        ((uint8_t*)(&Return))[i] = input.front();
        input.erase(input.begin());
    }

    return Return;
}

inline uint8_t code_buffer_pop8(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint8_t>(buffer);
}

inline uint16_t code_buffer_pop16(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint16_t>(buffer);
}

inline uint32_t code_buffer_pop32(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint32_t>(buffer);
}

inline uint64_t code_buffer_pop64(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint64_t>(buffer);
}

struct SYSDARFT_EXPORT_SYMBOL parsed_target_t
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

parsed_target_t encode_target(std::vector<uint8_t> &, const std::string&);
void SYSDARFT_EXPORT_SYMBOL encode_instruction(std::vector<uint8_t> &, const std::string &);
void decode_target(std::vector<std::string> &, std::vector<uint8_t> &);
void SYSDARFT_EXPORT_SYMBOL decode_instruction(std::vector < std::string > &,
    std::vector<uint8_t> &);

#endif // INSTRUCTIONS_H
