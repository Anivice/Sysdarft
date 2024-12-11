#include <debug.h>
#include <array>
#include <list>
#include <set>
#include <unordered_map>
#include <unordered_set>

int main()
{
    const std::array<int, 5> arr = {1, 2, 3, 4, 5};
    const std::list<int> list = {1, 2, 3, 4, 5};
    const std::vector<int> vec = {1, 2, 3, 4, 5};
    const std::set<int> set = {1, 2, 3, 4, 5};
    const std::unordered_map<int, int> umap = {{1, 2}, {3, 4}};
    const std::unordered_set<int> uset = {1, 2, 3, 4, 5};
    const std::map <std::string, int> map { { "one", 1 }, { "two", 2 } };
    debug::log("Hello World!\n");
    debug::log("Parameters: ", arr, " ", list, " ", vec, " ", set, " ", umap, " ", uset, " ", map, '\n');
    return 0;
}
