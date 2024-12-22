#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

// Instruction prefix
#define _8bit_instruction_prefix  0x08
#define _16bit_instruction_prefix 0x16
#define _32bit_instruction_prefix 0x32
#define _64bit_instruction_prefix 0x64

// Operand Prefix
#define OperandIsRegister   0x01
#define OperandIsMemory     0x02
#define OperandIsConstant   0x03

// Number Prefix
#define NUM_PREFIX '$'

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>

#define EXPORT __attribute__((visibility("default")))

struct EXPORT Number {
    enum Type { NaN, NumberType, FloatType } type { };
    union {
        __uint128_t unsigned_number;
        __int128_t signed_number;
        long double float_number;
    } number { };
};

Number EXPORT strToNumber(const std::string& str);
std::string EXPORT replaceAsterisks(const std::string& input, const std::string &target, const std::string &replacement);
std::vector<std::string> EXPORT lines_to_words(const std::string& input, const std::vector<char>& delimiters);
std::string EXPORT toUpperCaseTransform(const std::string& input);

// Initialize the instruction to opcode mapping
const std::unordered_map<std::string, std::map < std::string /* Entry Name */, uint64_t /* Entry Value */ >> instructionMap =
{
    {"NOP",     {   { "opcode", 0x00 }, { "argc", 0 }}  },
    {"ADD",     {   { "opcode", 0x01 }, { "argc", 2 }}  },
    {"ADC",     {   { "opcode", 0x02 }, { "argc", 2 }}  },
    {"SUB",     {   { "opcode", 0x03 }, { "argc", 2 }}  },
    {"SBB",     {   { "opcode", 0x04 }, { "argc", 2 }}  },
    {"MUL",     {   { "opcode", 0x05 }, { "argc", 1 }}  },
    {"IMUL",    {   { "opcode", 0x06 }, { "argc", 1 }}  },
    {"DIV",     {   { "opcode", 0x07 }, { "argc", 1 }}  },
    {"IDIV",    {   { "opcode", 0x08 }, { "argc", 1 }}  },
    {"NEG",     {   { "opcode", 0x09 }, { "argc", 1 }}  },
    {"CMP",     {   { "opcode", 0x0A }, { "argc", 2 }}  },
    {"AND",     {   { "opcode", 0x10 }, { "argc", 2 }}  },
    {"OR",      {   { "opcode", 0x11 }, { "argc", 2 }}  },
    {"XOR",     {   { "opcode", 0x12 }, { "argc", 2 }}  },
    {"NOT",     {   { "opcode", 0x13 }, { "argc", 1 }}  },
    {"SHL",     {   { "opcode", 0x14 }, { "argc", 2 }}  },
    {"SHR",     {   { "opcode", 0x15 }, { "argc", 2 }}  },
    {"SAL",     {   { "opcode", 0x16 }, { "argc", 2 }}  },
    {"SAR",     {   { "opcode", 0x17 }, { "argc", 2 }}  },
    {"ROR",     {   { "opcode", 0x18 }, { "argc", 2 }}  },
    {"ROL",     {   { "opcode", 0x19 }, { "argc", 2 }}  },
    {"RCL",     {   { "opcode", 0x1A }, { "argc", 2 }}  },
    {"RCR",     {   { "opcode", 0x1B }, { "argc", 2 }}  },
    {"MOV",     {   { "opcode", 0x20 }, { "argc", 2 }}  },
    {"LEA",     {   { "opcode", 0x21 }, { "argc", 2 }}  },
    {"XCHG",    {   { "opcode", 0x22 }, { "argc", 2 }}  },
    {"ENTER",   {   { "opcode", 0x23 }, { "argc", 1 }}  },
    {"PUSH",    {   { "opcode", 0x24 }, { "argc", 1 }}  },
    {"POP",     {   { "opcode", 0x25 }, { "argc", 1 }}  },
    {"MOVS",    {   { "opcode", 0x26 }, { "argc", 0 }}  },
    {"PUSHALL", {   { "opcode", 0x27 }, { "argc", 0 }}  },
    {"POPALL",  {   { "opcode", 0x28 }, { "argc", 0 }}  },
    {"PUSHF",   {   { "opcode", 0x29 }, { "argc", 0 }}  },
    {"POPF",    {   { "opcode", 0x2A }, { "argc", 0 }}  },
    {"LEAVE",   {   { "opcode", 0x2B }, { "argc", 0 }}  },
    {"JMP",     {   { "opcode", 0x30 }, { "argc", 1 }}  },
    {"CALL",    {   { "opcode", 0x31 }, { "argc", 1 }}  },
    {"RET",     {   { "opcode", 0x32 }, { "argc", 0 }}  },
    {"LOOP",    {   { "opcode", 0x33 }, { "argc", 1 }}  },
    {"JE",      {   { "opcode", 0x34 }, { "argc", 1 }}  },
    {"JNE",     {   { "opcode", 0x35 }, { "argc", 1 }}  },
    {"JB",      {   { "opcode", 0x36 }, { "argc", 1 }}  },
    {"JL",      {   { "opcode", 0x37 }, { "argc", 1 }}  },
    {"JBE",     {   { "opcode", 0x38 }, { "argc", 1 }}  },
    {"JLE",     {   { "opcode", 0x39 }, { "argc", 1 }}  },
    {"INT",     {   { "opcode", 0x3A }, { "argc", 1 }}  },
    {"INT3",    {   { "opcode", 0x3B }, { "argc", 0 }}  },
    {"IRET",    {   { "opcode", 0x3C }, { "argc", 0 }}  },
    {"FADD",    {   { "opcode", 0x40 }, { "argc", 2 }}  },
    {"FSUB",    {   { "opcode", 0x41 }, { "argc", 2 }}  },
    {"FMUL",    {   { "opcode", 0x42 }, { "argc", 1 }}  },
    {"FDIV",    {   { "opcode", 0x43 }, { "argc", 1 }}  },
    {"FLDI",    {   { "opcode", 0x44 }, { "argc", 2 }}  },
    {"FLDFT",   {   { "opcode", 0x45 }, { "argc", 2 }}  },
    {"FXCHG",   {   { "opcode", 0x46 }, { "argc", 2 }}  },
    {"HLT",     {   { "opcode", 0x50 }, { "argc", 0 }}  },
    {"RDTSCP",  {   { "opcode", 0x51 }, { "argc", 0 }}  },
    {"SYSCALL", {   { "opcode", 0x52 }, { "argc", 0 }}  },
    {"SYSRET",  {   { "opcode", 0x53 }, { "argc", 0 }}  },
    {"SVM",     {   { "opcode", 0x54 }, { "argc", 1 }}  },
    {"ENTVMFCXT", { { "opcode", 0x55 }, { "argc", 1 }}  },
    {"EXTVM",   {   { "opcode", 0x56 }, { "argc", 0 }}  },
    {"INS",     {   { "opcode", 0x60 }, { "argc", 2 }}  },
    {"OUTS",    {   { "opcode", 0x61 }, { "argc", 2 }}  },
    {"LOCK",    {   { "opcode", 0x70 }, { "argc", 1 }}  },
    {"UNLOCK",  {   { "opcode", 0x71 }, { "argc", 1 }}  },
};

#endif //INSTRUCTIONS_H
