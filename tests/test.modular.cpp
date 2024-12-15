#include <module.h>
#include <iostream>

int main()
{
    try {
        Module ExampleModule("modules/libexample.so");
        ExampleModule.call<void>("greet");

        try {
            ExampleModule.call<void>("hello");
        } catch (const ModuleResolutionError & err) {
            std::cout << err.what() << std::endl;
            return 0;
        }

        return 1;
    } catch (SysdarftBaseError & err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
}
