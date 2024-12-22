#include <instruction.h>
#include <iostream>

int main()
{
    // Define test cases
    std::string tests[] = {
        "12345",        // Base 10
        "-67890",       // Base 10 Negative
        "0x1A3F",       // Base 16
        "0XABCDEF",     // Base 16 Uppercase
        "abcdef",       // Not a Number (since no 0x prefix)
        "-0x1A3F",      // Not a Number (negative hex without proper handling)
        "123.456",      // Floating-point
        "   7890   ",   // Base 10 with Whitespaces
        "   -7890   ",  // Base 10 Negative with Whitespaces
        "0xGHIJ",       // Invalid Hex
        "",             // Empty String
        "-",            // Only Negative Sign
        "123e-5",       // Floating-point with Exponent
        "0x",           // Hex Prefix Only
        "0x123G45",     // Invalid Hex Character
        "12345abc",     // Mixed Base 10 and Letters
        "   ",          // Whitespaces Only
    };

    for (const auto& test_str : tests)
    {
        auto [type, number] = strToNumber(test_str);
        std::cout << "Input: \"" << test_str << "\"\n";

        switch (type)
        {
            case Number::NumberType:
                // Determine if it's Base 10 or Base 16 based on prefix
                if (test_str.size() >= 3 && test_str[0] == '0' && (test_str[1] == 'x' || test_str[1] == 'X')) {
                    // Base 16 Number
                    std::cout << "Type: Base 16 Number\n";
                    // Convert __uint128_t to hexadecimal string for display
                    std::string hex_str;
                    __uint128_t temp = number.unsigned_number;
                    if (temp == 0) {
                        hex_str = "0";
                    }
                    else
                    {
                        while (temp > 0)
                        {
                            char digit = temp % 16;
                            if (digit < 10)
                                hex_str = static_cast<char>('0' + digit) + hex_str;
                            else
                                hex_str = static_cast<char>('A' + (digit - 10)) + hex_str;
                            temp /= 16;
                        }
                    }
                    std::cout << "Value: 0x" << hex_str << "\n";
                } else {
                    // Base 10 Number
                    std::cout << "Type: Base 10 Number\n";
                    // Since __int128_t is not directly printable, we need to convert it to string
                    __int128_t value = number.signed_number;
                    bool is_negative = false;
                    if (value < 0) {
                        is_negative = true;
                        value = -value;
                    }

                    std::string num_str;
                    if (value == 0) {
                        num_str = "0";
                    } else {
                        while (value > 0) {
                            num_str = static_cast<char>('0' + (value % 10)) + num_str;
                            value /= 10;
                        }
                    }
                    if (is_negative) {
                        num_str = "-" + num_str;
                    }
                    std::cout << "Value: " << num_str << "\n";
                }
                break;

            case Number::FloatType:
                std::cout << "Type: Floating-point Number\n";
                std::cout << "Value: " << number.float_number << "\n";
                break;

            case Number::NaN:
            default:
                std::cout << "Type: Not a Number (NaN)\n";
                break;
        }

        std::cout << "--------------------------\n";
    }

    return 0;
}
