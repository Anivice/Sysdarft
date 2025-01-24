/* EncodingDecoding.h
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

/**
 * @file EncodingDecoding.h
 * @brief Utilities for encoding and decoding (Assembler and Disassembler)
 *
 * This file is a collection of tool used for assembling and disassembling files.
 *
 * This file defined:
 * - SysdarftCodeExpressionError
 * - SysdarftAssemblerError
 * - SysdarftInvalidArgument
 * - CodeBufferEmptiedWhenPop
 * - InstructionExpressionError
 * - SysdarftPreProcessorError
 * - remove_space()
 * - capitalization()
 * - code_buffer_push()
 * - code_buffer_push8()
 * - code_buffer_push16()
 * - code_buffer_push32()
 * - code_buffer_push64()
 * - code_buffer_pop()
 * - code_buffer_pop8()
 * - code_buffer_pop16()
 * - code_buffer_pop32()
 * - code_buffer_pop64()
 * - parsed_target_t
 * - bad_nbit()
 * - @ref defined_line_marker_t
 * - process_base16()
 * - replace_all()
 * - execute_bc()
 * - encode_target()
 * - decode_target()
 * - @ref target_pattern
 * - encode_instruction()
 * - decode_instruction()
 * - SysdarftCompile()
 * - CodeProcessing()
 */

/*! operand or operation width, 8bit */
#define _8bit_prefix  (0x08)
/*! operand or operation width, 16bit */
#define _16bit_prefix (0x16)
/*! operand or operation width, 32bit */
#define _32bit_prefix (0x32)
/*! operand or operation width, 64bit */
#define _64bit_prefix (0x64)

/*! operand is a register */
#define REGISTER_PREFIX (0x01)
/*! operand is a constant */
#define CONSTANT_PREFIX (0x02)
/*! operand is a memory location */
#define MEMORY_PREFIX   (0x03)

/*! Register code for Stack Base */
#define R_StackBase                     (0xA0)
/*! Register code for Stack Pointer */
#define R_StackPointer                  (0xA1)
/*! Register code for Code Base */
#define R_CodeBase                      (0xA2)
/*! Register code for Data Base */
#define R_DataBase                      (0xA3)
/*! Register code for Data Pointer */
#define R_DataPointer                   (0xA4)
/*! Register code for Extended Base */
#define R_ExtendedBase                  (0xA5)
/*! Register code for Extended Pointer */
#define R_ExtendedPointer               (0xA6)

#include <string>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <SysdarftDebug.h>

/*!
 * @brief Error when processing source code:
 *
 * Used by assembler to indicate that an unrecognized and unrecoverable error found in source code
 *
 * @callergraph
 * @callgraph
 *
 */
class SysdarftCodeExpressionError final : public SysdarftBaseError
{
public:
    /*!
     * @brief Create a SysdarftCodeExpressionError object
     *
     * Used by assembler to indicate that an unrecognized and unrecoverable error found in source code
     *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     * @return None
     *
     */
    explicit SysdarftCodeExpressionError(const std::string & message) :
        SysdarftBaseError("Cannot parse Target expression: " + message) { }
};

/*!
 * @brief Error when processing source code:
 *
 * Used by higher level assembler to indicate that an unrecognized and unrecoverable error found in source code
 *
 * @callergraph
 * @callgraph
 *
 */
class SysdarftAssemblerError final : public SysdarftBaseError
{
public:
    /*!
     * @brief Create a SysdarftAssemblerError object
     *
     * Used by higher level assembler to indicate that an unrecognized and unrecoverable error found in source code
     *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     * @return None
     *
     */
    explicit SysdarftAssemblerError(const std::string & message) :
        SysdarftBaseError("Assembler cannot parse the code expression: " + message) { }
};

/*!
 * @brief Generic invalid argument:
 *
 * Though being generic, it is only used by code_buffer_push to indicate an invalid argument
 *
 * @callergraph
 * @callgraph
 *
 */
class SysdarftInvalidArgument final : public SysdarftBaseError
{
public:
    /*!
     * @brief Generic invalid argument:
     *
     * Though being generic, it is only used by code_buffer_push to indicate an invalid argument
     *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     * @return None
     *
     */
    explicit SysdarftInvalidArgument(const std::string & message) :
        SysdarftBaseError("Invalid argument provided: " + message) { }
};

/*!
 * @brief Disassembler buffer emptied when attempting a pop operation:
 *
 * Often used in disassemblers.
 *
 * @callergraph
 * @callgraph
 *
 */
class CodeBufferEmptiedWhenPop final : public SysdarftBaseError {
public:
    /*!
     * @brief Disassembler buffer emptied when attempting a pop operation:
     *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     * @return None
     *
     */
    explicit CodeBufferEmptiedWhenPop(const std::string & message) :
        SysdarftBaseError("Code buffer emptied before pop finished: " + message) { }
};

/*!
 * @brief Assembler encountered error when parsing instruction in source code
 *
 * @callergraph
 * @callgraph
 *
 */
class InstructionExpressionError final : public SysdarftBaseError {
public:
    /*!
     * @brief Assembler encountered error when parsing instruction in source code
    *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     *
     * @return None
     *
     */
    explicit InstructionExpressionError(const std::string& message) :
        SysdarftBaseError("Instruction Expression Error: " + message) { }
};

/*!
 * @brief PreProcessor encountered error when parsing instruction in source code
 *
 * @callergraph
 * @callgraph
 *
 */
class SysdarftPreProcessorError final : public SysdarftBaseError {
public:
    /*!
     * @brief PreProcessor encountered error when parsing instruction in source code
     *
     * @callergraph
     * @callgraph
     *
     * @param message Error description
     *
     * @return None
     *
     */
    explicit SysdarftPreProcessorError(const std::string& message) :
        SysdarftBaseError("Error encountered in preprocessor: " + message) { }
};

/*!
 * @brief Remove all spaces in provided std::string
 *
 * @callergraph
 * @callgraph
 *
 * @param str String to process
 * @return Reference to processed string, which is the std::string provided in the parameter
 *
 */
inline std::string & remove_space(std::string & str)
{
    std::erase(str, ' ');
    return str;
}

/*!
 * @brief Function to convert a string to uppercase
 *
 * @callergraph
 * @callgraph
 *
 * @param input String to process
 * @return Reference to processed string, which is the std::string provided in the parameter
 *
 */
inline std::string & capitalization(std::string& input)
{
    std::ranges::transform(input, input.begin(),
                           [](const unsigned char c) { return std::toupper(c); });
    return input;
}

/*!
 * @brief push data to std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @tparam LENGTH data size, in bits, must be 8bit aligned
 * @param buffer Reference to buffer
 * @param value Pointer to value(data)
 * @return Nothing
 *
 * @throw SysdarftInvalidArgument
 *
 */
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

/*!
 * @brief push an 8-bit variable to std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @param value Pointer to value(data)
 * @return Nothing
 *
 * @throw SysdarftInvalidArgument
 *
 */
inline void code_buffer_push8(std::vector<uint8_t> & buffer, const uint8_t value) {
    code_buffer_push<8>(buffer, &value);
}

/*!
 * @brief push a 16bit variable to std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @param value Pointer to value(data)
 * @return Nothing
 *
 * @throw SysdarftInvalidArgument
 *
 */
inline void code_buffer_push16(std::vector<uint8_t> & buffer, const uint16_t value) {
    code_buffer_push<16>(buffer, &value);
}

/*!
 * @brief push a 32bit variable to std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @param value Pointer to value(data)
 * @return Nothing
 *
 * @throw SysdarftInvalidArgument
 *
 */
inline void code_buffer_push32(std::vector<uint8_t> & buffer, const uint32_t value) {
    code_buffer_push<32>(buffer, &value);
}

/*!
 * @brief push a 64bit variable to std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @param value Pointer to value(data)
 * @return Nothing
 *
 * @throw SysdarftInvalidArgument
 *
 */
inline void code_buffer_push64(std::vector<uint8_t> & buffer, const uint64_t value) {
    code_buffer_push<64>(buffer, &value);
}

/*!
 * @brief pop data from std::vector < uint8_t > type buffer from start, and erase the corresponding elements
 *
 * @callergraph
 * @callgraph
 *
 * Example usage:
 * @code
 * std::vector < uint8_t > data;
 * // some operations to prepare data
 * auto popped = code_buffer_pop<uint64_t>(data);
 * @endcode
 *
 * @tparam DataType Data type to be popped
 * @param input Reference to buffer
 * @return Popped data
 *
 * @throw CodeBufferEmptiedWhenPop
 *
 */
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

/*!
 * @brief pop an 8-bit variable from std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @return 8bit value
 *
 * @throw CodeBufferEmptiedWhenPop
 *
 */
inline uint8_t code_buffer_pop8(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint8_t>(buffer);
}

/*!
 * @brief pop a 16-bit variable from std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @return 16bit value
 *
 * @throw CodeBufferEmptiedWhenPop
 *
 */
inline uint16_t code_buffer_pop16(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint16_t>(buffer);
}

/*!
 * @brief pop a 32-bit variable from std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @return 32bit value
 *
 * @throw CodeBufferEmptiedWhenPop
 *
 */
inline uint32_t code_buffer_pop32(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint32_t>(buffer);
}

/*!
 * @brief pop a 64-bit variable from std::vector < uint8_t > type buffer
 *
 * @callergraph
 * @callgraph
 *
 * @param buffer Reference to buffer
 * @return 64bit value
 *
 * @throw CodeBufferEmptiedWhenPop
 *
 */
inline uint64_t code_buffer_pop64(std::vector<uint8_t> & buffer) {
    return code_buffer_pop<uint64_t>(buffer);
}

/*!
 * @brief parsed target type to store an operand (raw data from source code)
 * Whither it's a register, constant, memory, or a code offset reference
 *
 * @callergraph
 * @callgraph
 *
 */
struct parsed_target_t
{
    /// Operand type, can be either NOTaValidType (not a valid type), REGISTER, CONSTANT, MEMORY, or CODE_POSITION
    enum { NOTaValidType, REGISTER, CONSTANT, MEMORY, CODE_POSITION } TargetType { };

    /// Register name. Empty if not a register
    std::string RegisterName { };

    /// Constant expression, Empty if not a constant
    std::string ConstantExpression { };

    /// Memory reference expression, Empty if not a memory reference
    struct {
        /// Memory access ratio. Can be 1, 2, 4, or 16: *[Ratio]&BitWidth(Base Address, Offset 1, Offset 2);
        std::string MemoryAccessRatio;
        /// Memory base address *Ratio&BitWidth([Base Address], Offset 1, Offset 2);
        std::string MemoryBaseAddress;
        /// Memory offset 1 *Ratio&BitWidth(Base Address, [Offset 1], Offset 2);
        std::string MemoryOffset1;
        /// Memory offset 2 *Ratio&BitWidth(Base Address, Offset 1, [Offset 2]);
        std::string MemoryOffset2;
        /// Memory access width, can be 8, 16, 32, or 64. *Ratio&[BitWidth](Base Address, Offset 1, Offset 2);
        std::string MemoryWidth;
    } memory { };
};

/*!
 * @brief When disassembling a code, there are data that cannot fit into any instruction paradigm.
 * Instead of ignoring the data, print it out since it might be data.
 * If it is printable, show in ASCII code format, e.g., 'A',
 * otherwise, show in hexadecimal format, like 0xFF.
 * It can process more than one byte, and split them into 8bit byte codes.
 *
 * @callergraph
 * @callgraph
 *
 * @tparam Type Variable type
 * @tparam size Variable size, in bytes
 * @param data Data that cannot be processed. can have arbitrary length
 * @return Processed std::string
 *
 */
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

/// @brief Type that defines line marker, can have more than one
///
/// < <line marker name>, < <marker position>, <location it appeared> > >
typedef std::map < std::string, /* line marker name */
    std::pair < uint64_t /* marker position */,
    std::vector < uint64_t > /* location it appeared */ >
> defined_line_marker_t;

/*!
 * @brief process hexadecimal numbers appeared in a string, replace them with decimals
 *
 * @param expression Reference of the string to be processed
 * @return Nothing
 *
 */
void SYSDARFT_EXPORT_SYMBOL process_base16(std::string & expression);

/*!
 * @brief Replace all appearances of a substring in a string with another one
 *
 * @param original String to be processed
 * @param target Target to be replaced
 * @param replacement Replacement of target
 * @return Nothing
 *
 */
void SYSDARFT_EXPORT_SYMBOL replace_all(std::string & original,
    const std::string & target, const std::string & replacement);

/*!
 * @brief Processed the expression through `bc` calculator
 *
 * @param expression bc expression
 * @return result from bc, a decimal in string format
 *
 */
std::string SYSDARFT_EXPORT_SYMBOL execute_bc(const std::string & expression);

/*!
 * @brief Parse, and encode an operand
 *
 * @param buffer Reference of a std::vector < uint8_t > buffer
 * @param operand an operand expression
 * @return literal of parsed operand, with its format being parsed_target_t
 * @throw SysdarftCodeExpressionError
 */
parsed_target_t SYSDARFT_EXPORT_SYMBOL encode_target(std::vector < uint8_t > & buffer,
    const std::string & operand);

/*!
 * @brief Disassemble an operand, and push the result into literal_buffer
 *
 * @param literal_buffer Reference of a std::vector < uint8_t > buffer
 * @param code_buffer an operand expression
 * @return Nothing
 *
 */
void decode_target(std::vector < std::string > & literal_buffer,
    std::vector < uint8_t > & code_buffer);

/// Regular expression of the operand, captures '<' and '>' as well
const std::regex target_pattern(R"(<\s*(?:\*\s*(?:1|2|4|8|16)\&(8|16|32|64)\s*\([^,]+,[^,]+,[^,]+\)|%(?:R|EXR|HER)[0-7]|%(FER)([\d]+)|%(SB|SP|CB|DB|DP|EB|EP)|\$\s*\(\s*(?:0[xX][A-Fa-f0-9]+|\s|[+\-.',*\/^%()xX0-9-])+\s*\))\s*>)");

/*!
 * @brief Encode an instruction
 *
 * @param buffer Reference of a std::vector < uint8_t > buffer
 * @param instruction an instruction expression
 * @return Nothing
 * @throw InstructionExpressionError
 */
void encode_instruction(std::vector < uint8_t > & buffer,
    const std::string & instruction);

/*!
 * @brief Disassemble an instruction, and push the result into literal_buffer
 *
 * @param literal_buffer Reference of a std::vector < uint8_t > buffer
 * @param code_buffer an operand expression
 * @return Nothing
 *
 */
void SYSDARFT_EXPORT_SYMBOL decode_instruction(std::vector < std::string > & literal_buffer,
    std::vector<uint8_t> & code_buffer);

/*!
 * @brief Compile a sets of instructions from data stream
 *
 * @param instruction_buffer_set Reference of a std::vector < std::vector < uint8_t > > buffer.
 * Each element (std::vector < uint8_t >) of the outer vector is an encoded instruction
 * @param file Input data stream for compiling
 * @param origin Origin used for the compilation process (code offset of the current line markers)
 * @param appeared_line_markers All appearances of line markers, name is required, appearances and
 * its own location can be (and should be) left blank
 * @param line_number Starting of the line number of the file, since file might be part of a larger stream
 * @return Nothing
 *
 * @throw SysdarftAssemblerError
 *
 */
void
#ifdef __DEBUG__
    SYSDARFT_EXPORT_SYMBOL
#endif
SysdarftCompile(
    std::vector < std::vector < uint8_t > > & instruction_buffer_set,
    std::basic_iostream < char > & file,
    uint64_t origin,
    defined_line_marker_t & appeared_line_markers,
    uint64_t line_number = 0);

/**
 * @brief Compile a file from data stream.
 * This serves as a preprocessor of the assembly file
 *
 * @param code Encoded binary code
 * @param file Data stream, can be a file or any other data stream types
 * @param regex If regular expression is used in .equ preprocessor directive
 * @return Nothing
 *
 * @throw SysdarftPreProcessorError
 */
void SYSDARFT_EXPORT_SYMBOL CodeProcessing(
    std::vector < uint8_t > & code,
    std::basic_istream < char > & file,
    bool regex);

#endif // INSTRUCTIONS_H
