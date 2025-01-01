#include <cstdio>
#include <vector>
#include <string>
#include <atomic>

extern "C" {
    int __attribute__((visibility("default")))  module_init(void);
    void __attribute__((visibility("default"))) module_exit(void);
    void __attribute__((visibility("default"))) inc();
    int __attribute__((visibility("default"))) get();
}

std::atomic < int > c;

int module_init(void)
{
    printf("example initialization!\n");
    return 0;
}

void module_exit(void)
{
    printf("example exit!\n");
}

void inc()
{
    c.fetch_add(1, std::memory_order_relaxed);
}

int get()
{
    return c;
}
