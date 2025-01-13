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
        std::stringstream file;
        file << ".equ 'HDD_SEC_START', '0x137'                                                              " << std::endl;
        file << ".equ 'HDD_SEC_COUNT', '0x138'                                                              " << std::endl;
        file << ".equ 'HDD_IO', '0x139'                                                                     " << std::endl;
        file << ".lab _start, reads, puts                                                                   " << std::endl;
        file << ".org 0xC1800                                                                               " << std::endl;
        file << "jmp <%CB>, _start                                                                          " << std::endl;
        file << "reads:                                                                                     " << std::endl;
        file << "   pushall                                                                                 " << std::endl;
        file << "   out .64bit <$(HDD_SEC_START)>, <$(0)> ; sector read starts from 0                       " << std::endl;
        file << "   out .64bit <$(HDD_SEC_COUNT)>, <$(1)> ; read 1 sector                                   " << std::endl;
        file << "   mov .64bit <%FER0>, <$(512)> ; ins operation input 512 byte data, meaning one sector    " << std::endl;
        file << "   ins .64bit <$(HDD_IO)>                                                                  " << std::endl;
        file << "   popall                                                                                  " << std::endl;
        file << "   ret                                                                                     " << std::endl;
        file << "puts:                                                                                      " << std::endl;
        file << "   pushall                                                                                 " << std::endl;
        file << "   mov .16bit <%EXR0>, <$(0)>                                                              " << std::endl;
        file << "   xor .64bit <%DP>, <%DP>                                                                 " << std::endl;
        file << "   _loop:                                                                                  " << std::endl;
        file << "       mov .8bit <%R2>, <*1&8(%DP, $(0), $(0))>                                            " << std::endl;
        file << "       int <$(0x10)>                                                                       " << std::endl;
        file << "       add .64bit <%DP>, <$(1)>                                                            " << std::endl;
        file << "       add .16bit <%EXR0>, <$(1)>                                                          " << std::endl;
        file << "       cmp .8bit <*1&8(%DP, $(0), $(0))>, <$(0)>                                           " << std::endl;
        file << "       jne <%CB>, _loop                                                                    " << std::endl;
        file << "   popall                                                                                  " << std::endl;
        file << "   ret                                                                                     " << std::endl;
        file << "_start:                                                                                    " << std::endl;
        file << "   mov .64bit <%sp>, <$(0xFFF)>                                                            " << std::endl;
        file << "   mov .64bit <%sb>, <$(0xFFFF)>                                                           " << std::endl;
        file << "   call <%cb>, reads                                                                       " << std::endl;
        file << "   call <%cb>, puts                                                                        " << std::endl;
        file << "   jmp <%CB>, <$(@)>                                                                       " << std::endl;

        CodeProcessing(code, file);

        uint64_t off = BIOS_START;
        for (const auto & c : code) {
            write_memory(off++, (char*)&c, 1);
        }

        for (int i = 0; i < 4096; i++) {
            execute(0);
        }
    }
};

int main()
{
    debug::verbose = true;

    SysdarftCursesUI curses_ui;
    g_ui_initialize_install(curses_ui, initialize);
    g_ui_cleanup_install(curses_ui, cleanup);
    g_ui_set_cur_vsb_install(curses_ui, set_cursor_visibility);
    g_ui_teletype_install(curses_ui, teletype);
    g_ui_set_cursor_install(curses_ui, set_cursor);

    g_ui_initialize();
    Exec base;
    getch();
    g_ui_cleanup();
}
