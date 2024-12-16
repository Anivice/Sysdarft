#include <cstdio>
#include <vector>
#include <string>

extern "C" {
    int module_init(void);
    void module_exit(void);
    void greet(void);
    std::vector<std::string> module_dependencies(void);
}

std::vector<std::string> module_dependencies(void)
{
    return { "example_dep1", "example_dep2" };
}

int module_init(void)
{
    printf("example initialization!\n");
    return 0;
}

void module_exit(void)
{
    printf("example exit!\n");
}

void greet(void)
{
    printf("Hello, World!\n");
}
