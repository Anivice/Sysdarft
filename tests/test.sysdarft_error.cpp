#include <debug.h>

void f3(int)
{
    throw SysdarftBaseError("Base error test");
}

void f2(const int a)
{
    f3(a);
}

void f1(const int a)
{
    f2(a);
}

int main()
{
    try
    {
        f1(12);
    }
    catch (SysdarftBaseError& e)
    {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}
