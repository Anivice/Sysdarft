#include <cstdio>
#include <vector>
#include <string>

extern "C" {
    int module_init(void);
    void module_exit(void);
    std::vector<std::string> module_dependencies(void);
}

std::vector<std::string> module_dependencies(void)
{
    return { };
}

int module_init(void)
{
    printf("example_dep_of_dep1 initialization!\n");
    return 0;
}

void module_exit(void)
{
    printf("example_dep_of_dep1 exit!\n");
}
