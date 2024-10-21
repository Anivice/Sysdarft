#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <string>

std::vector < std::string > list_shared_libraries();
std::vector<std::string> find_containing_substring(const std::vector<std::string>&, const std::string&);

#endif //TOOLS_H
