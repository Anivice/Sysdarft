#include <iostream>
#include <string>
#include "instruction.h"
#include <cctype>
#include <stdexcept>

// Function to trim leading and trailing whitespaces
std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.length() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    size_t end = s.length();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(start, end - start);
}

// Function to check if a string is a valid hexadecimal number
bool isValidHex(const std::string& s)
{
    if (s.empty()) return false;
    for (const char ch : s) {
        if (!std::isxdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

// Function to check if a string is a valid base 10 number (optional leading '-')
bool isValidBase10(const std::string& s)
{
    if (s.empty()) return false;
    size_t start = 0;
    if (s[0] == '-') {
        if (s.length() == 1) return false; // Only '-' is invalid
        start = 1;
    }
    for (size_t i = start; i < s.length(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

// Function to parse a hexadecimal string to __uint128_t
__uint128_t parseHex(const std::string& s)
{
    __uint128_t result = 0;
    for (char ch : s) {
        result <<= 4; // Multiply by 16
        if (ch >= '0' && ch <= '9') {
            result += (ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            result += (ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
            result += (ch - 'A' + 10);
        } else {
            throw std::invalid_argument("Invalid hexadecimal character encountered.");
        }
    }
    return result;
}

// Function to parse a base10 string to __int128_t
__int128_t parseBase10(const std::string& s)
{
    __int128_t result = 0;
    for (char ch : s) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::invalid_argument("Invalid base10 character encountered.");
        }
        result = result * 10 + (ch - '0');
    }
    return result;
}

// Function to determine the type of number and parse accordingly
Number EXPORT strToNumber(const std::string& str)
{
    Number result { };
    result.type = Number::NaN; // Default to NaN

    // Step 1: Trim the input string
    const std::string s = trim(str);

    if (s.empty()) {
        // Empty string is NaN
        return result;
    }

    // Step 2: Check for Base 16 (Hexadecimal)
    if (s.size() >= 3 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
        if (const std::string hex_part = s.substr(2); isValidHex(hex_part))
        {
            try {
                const __uint128_t hex_value = parseHex(hex_part);
                result.type = Number::NumberType;
                result.number.unsigned_number = hex_value;
                return result;
            } catch (const std::invalid_argument& /* e */) {
                // Parsing failed, continue to check other types
            }
        }
    }

    // Step 3: Check for Base 10 (Decimal)
    if (isValidBase10(s))
    {
        try {
            bool is_negative = false;
            std::string digits = s;
            if (s[0] == '-') {
                is_negative = true;
                digits = s.substr(1);
            }
            __int128_t base10_value = parseBase10(digits);
            if (is_negative) {
                base10_value = -base10_value;
            }
            result.type = Number::NumberType;
            result.number.signed_number = base10_value;
            return result;
        } catch (const std::invalid_argument& /* e */) {
            // Parsing failed, continue to check other types
        }
    }

    // Step 4: Check for Floating-point Number
    try {
        size_t idx;
        const long double float_value = std::stold(s, &idx);
        if (idx == s.length()) { // Ensure entire string was parsed
            result.type = Number::FloatType;
            result.number.float_number = float_value;
            return result;
        }
    } catch ([[maybe_unused]] const std::invalid_argument& e) {
        // Not a float
    } catch (const std::out_of_range& /* e */) {
        // Float value out of range
    }

    // If none of the above, it's NaN
    return result;
}
