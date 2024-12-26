#include <debug.h>
#include <thread>

void f3(int)
{
    throw SysdarftBaseError("Base error test");
}

void f2(const int a)
{
    try {
        f3(a);
    } catch (SysdarftBaseError& e) {
        throw SysdarftBaseError("Error caught: " + std::string(e.what()));
    }
}

void f1(const int a)
{
    try {
        f2(a);
    } catch (SysdarftBaseError& e) {
        throw SysdarftBaseError("Error caught: " + std::string(e.what()));
    }
}

int main()
{
    std::thread Thread1([]() {
        std::this_thread::sleep_for(std::chrono::seconds(1000));
    });

    debug::verbose = true;
    try {
        f1(12);
    } catch (SysdarftBaseError& e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 1;
}
