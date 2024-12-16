#include "ModuleForest.h"
#include <set>
#include <debug.h>

// Recursive helper to build the tree from a given module
ModuleForest_::ModuleTree_ ModuleForest_::buildTree(const std::string &module,
                                                    const std::map<std::string, std::vector<std::string>> &graph)
{
    ModuleTree_ node;
    node.name = module;

    auto it = graph.find(module);
    if (it != graph.end()) {
        for (const auto &childModule : it->second) {
            node.children.push_back(buildTree(childModule, graph));
        }
    }

    return node;
}

ModuleForest_::ModuleForest_(const std::map<std::string, std::vector<std::string>> &module_list)
{
    // The input is already in a suitable form, so let's use it directly as our "graph"
    const auto &graph = module_list;

    // Determine the root modules:
    // A root module is one that does not appear as a dependency of another module.
    std::set<std::string> allModules;
    std::set<std::string> dependentModules;

    for (const auto &kv : graph) {
        allModules.insert(kv.first);
        for (const auto &dep : kv.second) {
            dependentModules.insert(dep);
        }
    }

    // Roots are allModules - dependentModules
    std::vector<std::string> rootModules;
    for (const auto &m : allModules) {
        if (dependentModules.find(m) == dependentModules.end()) {
            rootModules.push_back(m);
        }
    }

    // Construct the forest by building a tree from each root.
    for (const auto &rootModule : rootModules) {
        roots.push_back(buildTree(rootModule, graph));
    }
}

void ModuleForest_::computeMaxLength(unsigned int &max_length)
{
    // A recursive lambda to find the longest name
    std::function<void(const ModuleTree_ &)> findMax = [&](const ModuleTree_ &node)
    {
        if (node.name.size() > max_length) {
            max_length = (unsigned int)node.name.size();
        }
        for (const auto &child : node.children) {
            findMax(child);
        }
    };

    for (const auto &r : roots) {
        findMax(r);
    }
}

void ModuleForest_::printTree(const ModuleTree_ &node, unsigned int &max_length, const std::string &prefix, bool isLast)
{
    // Print the current node
    debug::log(prefix, (isLast ? "└─ " : "├─ "), node.name, "\n");

    // Prepare prefix for children
    const std::string childPrefix = prefix + (isLast ? "   " : "│  ");

    // Print each child
    for (std::size_t i = 0; i < node.children.size(); ++i) {
        const bool lastChild = (i == node.children.size() - 1);
        printTree(node.children[i], max_length, childPrefix, lastChild);
    }
}


void ModuleForest_::printForest()
{
    if (roots.empty()) {
        return;
    }

    unsigned int max_length = 0;
    computeMaxLength(max_length);

    for (std::size_t i = 0; i < roots.size(); ++i) {
        const bool lastRoot = (i == roots.size() - 1);
        printTree(roots[i], max_length, "", lastRoot);
    }
}
