#include <SysdarftDebug.h>
#include <SysdarftRegister.h>

class TestRegister : public SysdarftRegister {
public:
    TestRegister()
    {
        store<RegisterType, 0>(1);
        store<RegisterType, 1>(2);
        store<RegisterType, 2>(3);
        store<RegisterType, 3>(4);
        store<RegisterType, 4>(5);
        store<RegisterType, 5>(6);
        store<RegisterType, 6>(7);
        store<RegisterType, 7>(8);
        store<ExtendedRegisterType, 4>(1);
        store<ExtendedRegisterType, 5>(2);
        store<ExtendedRegisterType, 6>(3);
        store<ExtendedRegisterType, 7>(4);
        store<HalfExtendedRegisterType, 4>(2);
        store<HalfExtendedRegisterType, 5>(3);
        store<HalfExtendedRegisterType, 6>(4);
        store<HalfExtendedRegisterType, 7>(5);
        store<FullyExtendedRegisterType, 4>(12);
        store<FullyExtendedRegisterType, 5>(1);
        store<FullyExtendedRegisterType, 6>(2);
        store<FullyExtendedRegisterType, 7>(3);
        store<FullyExtendedRegisterType, 8>(4);
        store<FullyExtendedRegisterType, 9>(5);
        store<FullyExtendedRegisterType, 10>(6);
        store<FullyExtendedRegisterType, 11>(7);
        store<FullyExtendedRegisterType, 12>(8);
        store<FullyExtendedRegisterType, 13>(9);
        store<FullyExtendedRegisterType, 14>(10);
        store<FullyExtendedRegisterType, 15>(11);
        store<StackPointerType>(0xFFFF);
    }
};

int main()
{
    debug::verbose = true;
    TestRegister test;
}
