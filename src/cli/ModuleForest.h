#ifndef MODULEFOREST_H
#define MODULEFOREST_H

#include <string>
#include <vector>
#include <map>

class ModuleForest_
{
private:
    struct ModuleTree_ {
        std::string name;
        std::vector<ModuleTree_> children;
    };

    std::vector<ModuleTree_> roots;

    // Recursive helper to build the tree from a given module
    ModuleTree_ buildTree(const std::string &module,
                          const std::map<std::string, std::vector<std::string>> &graph);
    void computeMaxLength(unsigned int &max_length);
    void printTree(const ModuleTree_ &node, unsigned int &max_length, const std::string &prefix, bool isLast);

public:
    explicit ModuleForest_(const std::map<std::string, std::vector<std::string>> &module_list);
    void printForest();
};

#endif //MODULEFOREST_H
