#include <iomanip>
#include <EncodingDecoding.h>
#include <GlobalEvents.h>
#include <InstructionSet.h>
#include <SysdarftCursesUI.h>
#include <SysdarftInstructionExec.h>
#include <SysdarftHardDisk.h>

class Exec final : public SysdarftCPUInstructionExecutor {
public:
    Exec()
    {
        SysdarftCursesUI::initialize();
        device_list.emplace_back(std::make_unique<SysdarftHardDisk>("hda.img"));
        std::vector < uint8_t > code;
        std::fstream file("example.sysasm", std::ios::in);
        CodeProcessing(code, file);

        uint64_t off = BIOS_START;
        for (const auto & c : code) {
            write_memory(off++, (char*)&c, 1);
        }

        for (int i = 0; i < 400000; i++) {
            execute(0);
        }
    }

    ~Exec() override
    {
        SysdarftCursesUI::cleanup();
    }
};

int main()
{
    Exec base;
    sleep(1);
}
