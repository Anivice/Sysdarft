#include <debug.h>

void f3()
{
    throw SysdarftBaseError("Base error test", SYSDARFT_OK);
}

void f2()
{
    f3();
}

void f1()
{
    f2();
}

int main()
{
    try
    {
        f1();
    }
    catch (SysdarftBaseError& e)
    {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}
