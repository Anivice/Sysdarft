#include <iostream>
#include <sstream>
#include <instruction.h>

// Example usage
int main()
{
    std::vector<std::string> testInputs = {
        "_start:",
        "nop",
        "add .8 *1($0x1234, $0,$0) , %R1"
    };

    for (auto testStr : testInputs)
    {
        testStr = replaceAsterisks(testStr, "*", " * ");
        testStr = replaceAsterisks(testStr, ":", " : ");
        testStr = replaceAsterisks(testStr, "%", " % ");
        testStr = replaceAsterisks(testStr, "$", " $ ");

        std::vector<std::string> result = lines_to_words(testStr, {' ', ',', '(', ')'});
        std::cout << "Input: \"" << testStr << "\"\n";
        std::cout << "Output:\n";
        for (const auto& word : result) {
            std::cout << "\"" << word << "\"\n";
        }
        std::cout << "--------------------------\n";
    }

    return 0;
}
