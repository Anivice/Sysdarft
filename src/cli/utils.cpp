#include "cli_local.h"

std::vector<std::string> splitAndDiscardEmpty(const std::string& str)
{
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (getline(iss, token, ' ')) {
        if (!token.empty()) {  // This check ensures no empty strings are added
            tokens.push_back(token);
        }
    }

    return tokens;
}
