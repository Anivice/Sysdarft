#ifndef INSTRUCTION_DEFINITION_H
#define INSTRUCTION_DEFINITION_H

#include <unordered_map>
#include <map>
#include <string>
#include <cstdint>

#define ENTRY_OPCODE "opcode"
#define ENTRY_ARGUMENT_COUNT "argument_count"
#define ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION "require_operation_width_specification"

#define OPCODE_NOP      (0x00)
#define OPCODE_ADD      (0x01)
#define OPCODE_ADC      (0x02)
#define OPCODE_SUB      (0x03)
#define OPCODE_SBB      (0x04)
#define OPCODE_IMUL     (0x05)
#define OPCODE_MUL      (0x06)
#define OPCODE_IDIV     (0x07)
#define OPCODE_DIV      (0x08)
#define OPCODE_NEG      (0x09)
#define OPCODE_CMP      (0x0A)

#define OPCODE_AND      (0x10)
#define OPCODE_OR       (0x11)
#define OPCODE_XOR      (0x12)
#define OPCODE_NOT      (0x13)

#define OPCODE_MOV      (0x20)

// Initialize the instruction to opcode mapping
const std::unordered_map<std::string, std::map<std::string, uint64_t>> instruction_map = {
    {"NOP", {
         {ENTRY_OPCODE, OPCODE_NOP},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"ADD", {
         {ENTRY_OPCODE, OPCODE_ADD},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ADC", {
         {ENTRY_OPCODE, OPCODE_ADC},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SUB", {
         {ENTRY_OPCODE, OPCODE_SUB},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SBB", {
         {ENTRY_OPCODE, OPCODE_SBB},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"IMUL", {
         {ENTRY_OPCODE, OPCODE_IMUL},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"MUL", {
         {ENTRY_OPCODE, OPCODE_MUL},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"IDIV", {
         {ENTRY_OPCODE, OPCODE_IDIV},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"DIV", {
         {ENTRY_OPCODE, OPCODE_DIV},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"NEG", {
         {ENTRY_OPCODE, OPCODE_NEG},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"CMP", {
         {ENTRY_OPCODE, OPCODE_CMP},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    ////////////////////////////////////////////////////////////////////////////////////////////

    {"AND", {
         {ENTRY_OPCODE, OPCODE_AND},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"OR", {
         {ENTRY_OPCODE, OPCODE_OR},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"XOR", {
         {ENTRY_OPCODE, OPCODE_XOR},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"NOT", {
         {ENTRY_OPCODE, OPCODE_NOT},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SHL", {
         {ENTRY_OPCODE, 0x14},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SHR", {
         {ENTRY_OPCODE, 0x15},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SAL", {
         {ENTRY_OPCODE, 0x16},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SAR", {
         {ENTRY_OPCODE, 0x17},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ROR", {
         {ENTRY_OPCODE, 0x18},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ROL", {
         {ENTRY_OPCODE, 0x19},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"RCL", {
         {ENTRY_OPCODE, 0x1A},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"RCR", {
         {ENTRY_OPCODE, 0x1B},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"MOV", {
         {ENTRY_OPCODE, 0x20},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"LEA", {
         {ENTRY_OPCODE, 0x21},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"XCHG", {
         {ENTRY_OPCODE, 0x22},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ENTER", {
         {ENTRY_OPCODE, 0x23},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"PUSH", {
         {ENTRY_OPCODE, 0x24},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"POP", {
         {ENTRY_OPCODE, 0x25},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"MOVS", {
         {ENTRY_OPCODE, 0x26},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"PUSHALL", {
         {ENTRY_OPCODE, 0x27},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"POPALL", {
         {ENTRY_OPCODE, 0x28},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"PUSHF", {
         {ENTRY_OPCODE, 0x29},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"POPF", {
         {ENTRY_OPCODE, 0x2A},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"LEAVE", {
         {ENTRY_OPCODE, 0x2B},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JMP", {
         {ENTRY_OPCODE, 0x30},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"CALL", {
         {ENTRY_OPCODE, 0x31},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"RET", {
         {ENTRY_OPCODE, 0x32},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"LOOP", {
         {ENTRY_OPCODE, 0x33},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JE", {
         {ENTRY_OPCODE, 0x34},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JNE", {
         {ENTRY_OPCODE, 0x35},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JB", {
         {ENTRY_OPCODE, 0x36},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JL", {
         {ENTRY_OPCODE, 0x37},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JBE", {
         {ENTRY_OPCODE, 0x38},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"JLE", {
         {ENTRY_OPCODE, 0x39},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"INT", {
         {ENTRY_OPCODE, 0x3A},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"INT3", {
         {ENTRY_OPCODE, 0x3B},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"IRET", {
         {ENTRY_OPCODE, 0x3C},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FADD", {
         {ENTRY_OPCODE, 0x40},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FSUB", {
         {ENTRY_OPCODE, 0x41},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FMUL", {
         {ENTRY_OPCODE, 0x42},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FDIV", {
         {ENTRY_OPCODE, 0x43},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FLDI", {
         {ENTRY_OPCODE, 0x44},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FLDFT", {
         {ENTRY_OPCODE, 0x45},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"FXCHG", {
         {ENTRY_OPCODE, 0x46},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"HLT", {
         {ENTRY_OPCODE, 0x50},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"RDTSCP", {
         {ENTRY_OPCODE, 0x51},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"SYSCALL", {
         {ENTRY_OPCODE, 0x52},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"SYSRET", {
         {ENTRY_OPCODE, 0x53},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"SVM", {
         {ENTRY_OPCODE, 0x54},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ENTVMFCXT", {
         {ENTRY_OPCODE, 0x55},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"EXTVM", {
         {ENTRY_OPCODE, 0x56},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"ENTPRTMD", {
         {ENTRY_OPCODE, 0x57},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"SETPRT", {
         {ENTRY_OPCODE, 0x58},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SETBND", {
         {ENTRY_OPCODE, 0x59},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"INS", {
         {ENTRY_OPCODE, 0x60},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"OUTS", {
         {ENTRY_OPCODE, 0x61},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"LOCK", {
         {ENTRY_OPCODE, 0x70},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"UNLOCK", {
         {ENTRY_OPCODE, 0x71},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },
};

#endif //INSTRUCTION_DEFINITION_H