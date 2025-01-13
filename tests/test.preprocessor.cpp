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
        device_list.emplace_back(std::make_unique<SysdarftHardDisk>("hda.img"));
        std::vector < uint8_t > code;
        std::fstream file("example.sysasm", std::ios::in);
        CodeProcessing(code, file);

        uint64_t off = BIOS_START;
        for (const auto & c : code) {
            write_memory(off++, (char*)&c, 1);
        }

        for (int i = 0; i < 20000; i++) {
            execute(0);
        }
    }
};

int main()
{
    SysdarftCursesUI curses_ui;
    g_ui_initialize_install(curses_ui, initialize);
    g_ui_cleanup_install(curses_ui, cleanup);
    g_ui_set_cur_vsb_install(curses_ui, set_cursor_visibility);
    g_ui_teletype_install(curses_ui, teletype);
    g_ui_set_cursor_install(curses_ui, set_cursor);

    g_ui_initialize();
    Exec base;
    sleep(1);
    g_ui_cleanup();
}
