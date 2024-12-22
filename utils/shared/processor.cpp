#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include "instruction.h"

std::string toUpperCaseTransform(const std::string& input)
{
    std::string output = input; // Create a copy to preserve the original string
    std::ranges::transform(output, output.begin(),
                           [](const unsigned char c) { return std::toupper(c); });
    return output;
}

// Function to check if a character is a delimiter
bool isDelimiter(const char ch, const std::vector<char>& delimiters)
{
    for (const char delimiter : delimiters) {
        if (ch == delimiter) {
            return true;
        }
    }
    return false;
}

std::string EXPORT replaceAsterisks(const std::string& input, const std::string &target, const std::string &replacement)
{
    std::string output = input;
    size_t pos = 0;
    while ((pos = output.find(target, pos)) != std::string::npos) {
        output.replace(pos, target.length(), replacement);
        pos += replacement.length(); // Move past the replacement
    }

    return output;
}

// Polished Function to split the input string into words based on delimiters
std::vector<std::string> EXPORT lines_to_words(const std::string& input, const std::vector<char>& delimiters)
{
    std::vector<std::string> words;
    std::string currentWord;
    size_t i = 0;
    const size_t n = input.length();

    // Iterate through the string until ';' is found or end of string
    while (i < n && input[i] != ';')
    {
        if (isDelimiter(input[i], delimiters))
        {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord.clear();
            }
            // Skip consecutive delimiters
            while (i < n && isDelimiter(input[i], delimiters)) {
                i++;
            }
        } else {
            currentWord += input[i];
            i++;
        }
    }

    // Add the last word if any
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }

    return words;
}
