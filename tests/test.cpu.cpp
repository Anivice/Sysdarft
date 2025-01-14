#include <fstream>
#include <EncodingDecoding.h>
#include <SysdarftCPU.h>

int main()
{
    std::vector < uint8_t > code;
    std::fstream file("example.sysasm", std::ios::in);
    CodeProcessing(code, file);
    SysdarftCPU cpu(32 * 1024 * 1024, code, "hda.img", "", "");
    cpu.Boot();
}
