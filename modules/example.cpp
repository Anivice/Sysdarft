#include <cstdio>

extern "C" {
    int module_init(void);
    void module_exit(void);
    void greet(void);
}

int module_init(void)
{
    printf("Module initialization!\n");
    return 0;
}

void module_exit(void)
{
    printf("Module exit!\n");
}

void greet(void)
{
    printf("Hello, World!\n");
}
