#include <iomanip>
#include <EncodingDecoding.h>
#include <GlobalEvents.h>
#include <InstructionSet.h>
#include <SysdarftCursesUI.h>
#include <SysdarftInstructionExec.h>
#include <SysdarftDisks.h>

int main()
{
    std::vector < std::string > file = {
        ".equ 'IO', '0x137'             ",
        ".org 0x7c00                    ",
        "_start:                        ",
        "jmp <%cb>, < _start >          ",
        ".16bit_data < _start + 0x14 >  "
    };

    std::vector < std::string > file2 = {
        ".lab _start                    ",
        "jmp <%cb>, < _start >          ",
        ".16bit_data < _start + 0x14 >  "
    };

    uint64_t org = 0;
    defined_line_marker_t symbol1, symbol2;
    std::vector< std::vector< uint8_t > > data, data2;

    PreProcess(file, symbol1, org, false);
    auto object = SysdarftAssemble(data, file, org, symbol1);
    PreProcess(file2, symbol2, org, false);
    auto object2 = SysdarftAssemble(data2, file2, org, symbol2);

    std::vector <object_t> objects;
    objects.push_back(object);
    objects.push_back(object2);

    object_t linked_object = SysdarftLink(objects);
}
