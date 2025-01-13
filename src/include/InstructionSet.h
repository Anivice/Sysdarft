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
#define OPCODE_SHL      (0x14)
#define OPCODE_SHR      (0x15)
#define OPCODE_ROL      (0x16)
#define OPCODE_ROR      (0x17)
#define OPCODE_RCL      (0x18)
#define OPCODE_RCR      (0x19)

#define OPCODE_MOV      (0x20)
#define OPCODE_XCHG     (0x21)
#define OPCODE_PUSH     (0x22)
#define OPCODE_POP      (0x23)
#define OPCODE_PUSHALL  (0x24)
#define OPCODE_POPALL   (0x25)
#define OPCODE_ENTER    (0x26)
#define OPCODE_LEAVE    (0x27)
#define OPCODE_MOVS     (0x28)
#define OPCODE_LEA      (0x29)

#define OPCODE_JMP      (0x30)
#define OPCODE_CALL     (0x31)
#define OPCODE_RET      (0x32)
#define OPCODE_JE       (0x33)
#define OPCODE_JNE      (0x34)
#define OPCODE_JB       (0x35)
#define OPCODE_JL       (0x36)
#define OPCODE_JBE      (0x37)
#define OPCODE_JLE      (0x38)
#define OPCODE_INT      (0x39)
#define OPCODE_INT3     (0x3A)
#define OPCODE_IRET     (0x3B)

#define OPCODE_HLT      (0x40)

#define OPCODE_IN       (0x50)
#define OPCODE_OUT      (0x51)
#define OPCODE_INS      (0x52)
#define OPCODE_OUTS     (0x53)

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
         {ENTRY_OPCODE, OPCODE_SHL},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"SHR", {
         {ENTRY_OPCODE, OPCODE_SHR},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"ROL", {
                 {ENTRY_OPCODE, OPCODE_ROL},
                 {ENTRY_ARGUMENT_COUNT, 2},
                 {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
             }
    },

    {"ROR", {
         {ENTRY_OPCODE, OPCODE_ROR},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"RCL", {
         {ENTRY_OPCODE, OPCODE_RCL},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"RCR", {
         {ENTRY_OPCODE, OPCODE_RCR},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    ////////////////////////////////////////////////////////////////////////////////////////////

    {"MOV", {
         {ENTRY_OPCODE, OPCODE_MOV},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"XCHG", {
         {ENTRY_OPCODE, OPCODE_XCHG},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"PUSH", {
         {ENTRY_OPCODE, OPCODE_PUSH},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"POP", {
         {ENTRY_OPCODE, OPCODE_POP},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    {"PUSHALL", {
         {ENTRY_OPCODE, OPCODE_PUSHALL},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"POPALL", {
         {ENTRY_OPCODE, OPCODE_POPALL},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"ENTER", {
                 {ENTRY_OPCODE, OPCODE_ENTER},
                 {ENTRY_ARGUMENT_COUNT, 1},
                 {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
             }
    },

    {"LEAVE", {
         {ENTRY_OPCODE, OPCODE_LEAVE},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"MOVS", {
                 {ENTRY_OPCODE, OPCODE_MOVS},
                 {ENTRY_ARGUMENT_COUNT, 0},
                 {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
             }
    },

    {"LEA", {
                 {ENTRY_OPCODE, OPCODE_LEA},
                 {ENTRY_ARGUMENT_COUNT, 2},
                 {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
             }
    },

    ////////////////////////////////////////////////////////////////////////////////////////////

    {"JMP", {
         {ENTRY_OPCODE, OPCODE_JMP},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"CALL", {
         {ENTRY_OPCODE, OPCODE_CALL},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"RET", {
         {ENTRY_OPCODE, OPCODE_RET},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JE", {
         {ENTRY_OPCODE, OPCODE_JE},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JNE", {
         {ENTRY_OPCODE, OPCODE_JNE},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JB", {
         {ENTRY_OPCODE, OPCODE_JB},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JL", {
         {ENTRY_OPCODE, OPCODE_JL},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JBE", {
         {ENTRY_OPCODE, OPCODE_JBE},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"JLE", {
         {ENTRY_OPCODE, OPCODE_JLE},
         {ENTRY_ARGUMENT_COUNT, 2},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"INT", {
         {ENTRY_OPCODE, OPCODE_INT},
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"INT3", {
         {ENTRY_OPCODE, OPCODE_INT3},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    {"IRET", {
         {ENTRY_OPCODE, OPCODE_IRET},
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    ////////////////////////////////////////////////////////////////////////////////////////////

    { "HLT", {
         {ENTRY_OPCODE, OPCODE_HLT },
         {ENTRY_ARGUMENT_COUNT, 0},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 0},
     }
    },

    ////////////////////////////////////////////////////////////////////////////////////////////

    { "IN", {
           {ENTRY_OPCODE, OPCODE_IN },
           {ENTRY_ARGUMENT_COUNT, 2},
           {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
       }
    },

    { "OUT", {
           {ENTRY_OPCODE, OPCODE_OUT },
           {ENTRY_ARGUMENT_COUNT, 2},
           {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
       }
    },

    { "INS", {
         {ENTRY_OPCODE, OPCODE_INS },
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },

    { "OUTS", {
         {ENTRY_OPCODE, OPCODE_OUTS },
         {ENTRY_ARGUMENT_COUNT, 1},
         {ENTRY_REQUIRE_OPERATION_WIDTH_SPECIFICATION, 1},
     }
    },
};

#endif //INSTRUCTION_DEFINITION_H