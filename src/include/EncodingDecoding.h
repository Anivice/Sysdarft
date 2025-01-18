#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Width
#define _8bit_prefix  (0x08)
#define _16bit_prefix (0x16)
#define _32bit_prefix (0x32)
#define _64bit_prefix (0x64)

// Prefix
#define REGISTER_PREFIX (0x01)
#define CONSTANT_PREFIX (0x02)
#define MEMORY_PREFIX   (0x03)

// Special Registers
#define R_StackBase                     (0xA0)
#define R_StackPointer                  (0xA1)
#define R_CodeBase                      (0xA2)
#define R_DataBase                      (0xA3)
#define R_DataPointer                   (0xA4)
#define R_ExtendedBase                  (0xA5)
#define R_ExtendedPointer               (0xA6)

#include <string>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <SysdarftDebug.h>

class SysdarftCodeExpressionError final : public SysdarftBaseError
{
public:
    explicit SysdarftCodeExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

class SysdarftAssemblerError final : public SysdarftBaseError
{
public:
    explicit SysdarftAssemblerError(const std::string & message) :
        SysdarftBaseError("Assembler cannot parse the code expression: " + message) { }
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

class InstructionExpressionError final : public SysdarftBaseError {
public:
    explicit InstructionExpressionError(const std::string& message) :
        SysdarftBaseError("Instruction Expression Error: " + message) { }
};

class SysdarftPreProcessorError final : public SysdarftBaseError {
public:
    explicit SysdarftPreProcessorError(const std::string& msg) :
        SysdarftBaseError("Error encountered in preprocessor: " + msg) { }
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
    enum { NOTaValidType, REGISTER, CONSTANT, MEMORY, CODE_POSITION } TargetType { };
    std::string RegisterName { };
    std::string ConstantExpression { };

    struct {
        std::string MemoryAccessRatio;
        std::string MemoryBaseAddress;
        std::string MemoryOffset1;
        std::string MemoryOffset2;
        std::string MemoryWidth;
    } memory { };
};

template < typename Type, unsigned size = sizeof(Type) >
std::string bad_nbit(const Type & data)
{
    std::stringstream ss;
    const auto cdata = (const char *)(&data);
    ss << ".8bit_data <";

    for (unsigned i = 0; i < size; i++)
    {
        if (std::isprint(cdata[i])) {
            ss << "\'" << cdata[i] << "\'";
        } else {
            ss  << "0x" << std::setw(2)
                << std::setfill('0')
                << std::uppercase << std::hex
                << static_cast<int>(*(unsigned char *)(cdata + i));
        }

        // only append when more than one byte
        if (size > 1) {
            ss << ", ";
        }
    }

    ss << ">";
    auto str = ss.str();
    return ss.str();
}

typedef std::map < std::string, std::pair < uint64_t /* line position */, std::vector < uint64_t > > > defined_line_marker_t;

void SYSDARFT_EXPORT_SYMBOL process_base16(std::string &);
void SYSDARFT_EXPORT_SYMBOL replace_all(std::string &, const std::string &, const std::string &);
std::string SYSDARFT_EXPORT_SYMBOL execute_bc(const std::string &);

parsed_target_t SYSDARFT_EXPORT_SYMBOL encode_target(std::vector < uint8_t > &, const std::string &);
void decode_target(std::vector < std::string > &, std::vector < uint8_t > &);

const std::regex target_pattern(R"(<\s*(?:\*\s*(?:1|2|4|8|16)\&(8|16|32|64)\s*\([^,]+,[^,]+,[^,]+\)|%(?:R|EXR|HER)[0-7]|%(FER)([\d]+)|%(SB|SP|CB|DB|DP|EB|EP)|%XMM[0-5]|\$\s*\(\s*(?:0[xX][A-Fa-f0-9]+|\s|[+\-.',*\/^%()xX0-9-])+\s*\))\s*>)");
void SYSDARFT_EXPORT_SYMBOL encode_instruction(std::vector < uint8_t > &, const std::string &);
void SYSDARFT_EXPORT_SYMBOL decode_instruction(std::vector < std::string > &, std::vector<uint8_t> &);
void SYSDARFT_EXPORT_SYMBOL SysdarftCompile(std::vector < std::vector < uint8_t > > &,
    std::basic_iostream < char > &,
    uint64_t, defined_line_marker_t &,
    uint64_t line_number = 0);
void SYSDARFT_EXPORT_SYMBOL CodeProcessing(std::vector < uint8_t > &, std::basic_istream < char > &);

#endif // INSTRUCTIONS_H
