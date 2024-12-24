#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Instruction prefix
#define _8bit_prefix  0x08
#define _16bit_prefix 0x16
#define _32bit_prefix 0x32
#define _64bit_prefix 0x64

// Prefix
#define REGISTER_PREFIX 0x01
#define MEMORY_PREFIX   0x02
#define CONSTANT_PREFIX 0x03

#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <any>

#define EXPORT __attribute__((visibility("default")))

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
    } memory;
};

#define ENTRY_OPCODE "opcode"
#define ENTRY_ARGUMENT_COUNT "argument_count"
#define ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION "require_operation_width_specification"

// Initialize the instruction to opcode mapping
const std::unordered_map<std::string, std::map<std::string, uint64_t>> instruction_map = {
    { "NOP", {
            { ENTRY_OPCODE, 0x00 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "ADD", {
            { ENTRY_OPCODE, 0x01 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "ADC", {
            { ENTRY_OPCODE, 0x02 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SUB", {
            { ENTRY_OPCODE, 0x03 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SBB", {
            { ENTRY_OPCODE, 0x04 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "MUL", {
            { ENTRY_OPCODE, 0x05 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "IMUL", {
            { ENTRY_OPCODE, 0x06 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "DIV", {
            { ENTRY_OPCODE, 0x07 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "IDIV", {
            { ENTRY_OPCODE, 0x08 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "NEG", {
            { ENTRY_OPCODE, 0x09 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "CMP", {
            { ENTRY_OPCODE, 0x0A },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "AND", {
            { ENTRY_OPCODE, 0x10 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "OR",  {
            { ENTRY_OPCODE, 0x11 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "XOR", {
            { ENTRY_OPCODE, 0x12 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "NOT", {
            { ENTRY_OPCODE, 0x13 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SHL", {
            { ENTRY_OPCODE, 0x14 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SHR", {
            { ENTRY_OPCODE, 0x15 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SAL", {
            { ENTRY_OPCODE, 0x16 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SAR", {
            { ENTRY_OPCODE, 0x17 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "ROR", {
            { ENTRY_OPCODE, 0x18 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "ROL", {
            { ENTRY_OPCODE, 0x19 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "RCL", {
            { ENTRY_OPCODE, 0x1A },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "RCR", {
            { ENTRY_OPCODE, 0x1B },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "MOV", {
            { ENTRY_OPCODE, 0x20 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "LEA", {
            { ENTRY_OPCODE, 0x21 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "XCHG", {
            { ENTRY_OPCODE, 0x22 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "ENTER", {
            { ENTRY_OPCODE, 0x23 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "PUSH", {
            { ENTRY_OPCODE, 0x24 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "POP", {
            { ENTRY_OPCODE, 0x25 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "MOVS", {
            { ENTRY_OPCODE, 0x26 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "PUSHALL", {
            { ENTRY_OPCODE, 0x27 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "POPALL", {
            { ENTRY_OPCODE, 0x28 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "PUSHF", {
            { ENTRY_OPCODE, 0x29 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "POPF", {
            { ENTRY_OPCODE, 0x2A },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "LEAVE", {
            { ENTRY_OPCODE, 0x2B },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "JMP", {
            { ENTRY_OPCODE, 0x30 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "CALL", {
            { ENTRY_OPCODE, 0x31 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "RET", {
            { ENTRY_OPCODE, 0x32 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "LOOP", {
            { ENTRY_OPCODE, 0x33 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JE", {
            { ENTRY_OPCODE, 0x34 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JNE", {
            { ENTRY_OPCODE, 0x35 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JB", {
            { ENTRY_OPCODE, 0x36 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JL", {
            { ENTRY_OPCODE, 0x37 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JBE", {
            { ENTRY_OPCODE, 0x38 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "JLE", {
            { ENTRY_OPCODE, 0x39 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "INT", {
            { ENTRY_OPCODE, 0x3A },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "INT3", {
            { ENTRY_OPCODE, 0x3B },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "IRET", {
            { ENTRY_OPCODE, 0x3C },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "FADD", {
            { ENTRY_OPCODE, 0x40 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FSUB", {
            { ENTRY_OPCODE, 0x41 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FMUL", {
            { ENTRY_OPCODE, 0x42 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FDIV", {
            { ENTRY_OPCODE, 0x43 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FLDI", {
            { ENTRY_OPCODE, 0x44 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FLDFT", {
            { ENTRY_OPCODE, 0x45 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "FXCHG", {
            { ENTRY_OPCODE, 0x46 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "HLT", {
            { ENTRY_OPCODE, 0x50 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "RDTSCP", {
            { ENTRY_OPCODE, 0x51 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "SYSCALL", {
            { ENTRY_OPCODE, 0x52 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "SYSRET", {
            { ENTRY_OPCODE, 0x53 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "SVM", {
            { ENTRY_OPCODE, 0x54 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "ENTVMFCXT", {
            { ENTRY_OPCODE, 0x55 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "EXTVM", {
            { ENTRY_OPCODE, 0x56 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "ENTPRTMD", {
            { ENTRY_OPCODE, 0x57 },
            { ENTRY_ARGUMENT_COUNT, 0 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0 },
        }
    },

    { "SETPRT", {
            { ENTRY_OPCODE, 0x58 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "SETBND", {
            { ENTRY_OPCODE, 0x59 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "INS", {
            { ENTRY_OPCODE, 0x60 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "OUTS", {
            { ENTRY_OPCODE, 0x61 },
            { ENTRY_ARGUMENT_COUNT, 2 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "LOCK", {
            { ENTRY_OPCODE, 0x70 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },

    { "UNLOCK", {
            { ENTRY_OPCODE, 0x71 },
            { ENTRY_ARGUMENT_COUNT, 1 },
            { ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1 },
        }
    },
};

parsed_target_t encode_target(std::vector<uint8_t> & buffer, const std::string& input);
void decode_target(std::vector<std::string> & output, std::vector<uint8_t> & input);

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

inline void push8(std::vector<uint8_t> & buffer, const uint8_t value)
{
    push<8>(buffer, &value);
}

inline void push16(std::vector<uint8_t> & buffer, const uint16_t value)
{
    push<16>(buffer, &value);
}

inline void push32(std::vector<uint8_t> & buffer, const uint32_t value)
{
    push<32>(buffer, &value);
}

inline void push64(std::vector<uint8_t> & buffer, const uint64_t value)
{
    push<64>(buffer, &value);
}

class TargetExpressionError final : public SysdarftBaseError
{
public:
    explicit TargetExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

template < unsigned int LENGTH >
std::any pop(std::vector<uint8_t> & input)
{
    static_assert(LENGTH % 8 == 0);

    __uint128_t result = 0;
    auto* buffer = reinterpret_cast<uint8_t *>(&result);

    for (unsigned int i = 0; i < LENGTH / 8; i++) {
        buffer[i] = input[0];
        input.erase(input.begin());
    }

    switch (LENGTH / 8)
    {
    case 1: /* 8bit */  return static_cast<uint8_t> (result & 0xFF);
    case 2: /* 16bit */ return static_cast<uint16_t>(result & 0xFFFF);
    case 4: /* 32bit */ return static_cast<uint32_t>(result & 0xFFFFFFFF);
    case 8: /* 64bit */ return static_cast<uint64_t>(result & 0xFFFFFFFFFFFFFFFF);
    default: throw TargetExpressionError("Unrecognized length");
    }
}

inline uint8_t pop8(std::vector<uint8_t> & input) {
    return std::any_cast<uint8_t>(pop<8>(input));
}

inline uint16_t pop16(std::vector<uint8_t> & input) {
    return std::any_cast<uint16_t>(pop<16>(input));
}

inline uint32_t pop32(std::vector<uint8_t> & input) {
    return std::any_cast<uint32_t>(pop<32>(input));
}

inline uint64_t pop64(std::vector<uint8_t> & input) {
    return std::any_cast<uint64_t>(pop<64>(input));
}

#endif //INSTRUCTIONS_H
