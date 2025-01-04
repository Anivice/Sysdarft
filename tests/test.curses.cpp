#include <SysdarftCursesUI.h>

char vm[V_WIDTH * V_HEIGHT] { };

int main(int argc, char **)
{
    SysdarftCursesUI curses;
    curses.register_vm(vm);
    curses.initialize();
    vm[18] = 'S';
    curses.commit_changes();
    curses.cleanup();
    g_get_input_install(curses, get_input);
    // g_get_input();
    return EXIT_SUCCESS;
}
