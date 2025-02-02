// ExampleUsage.cpp
#include <ASCIIKeymap.h>
#include <stdio.h>
#include <unistd.h>

extern enum KeyCode read_keyControl();

[[noreturn]]
int main()
{
    while (true)
    {
        usleep(1000);
        const int k = read_keyControl();
        if (k == NO_KEY) {
            continue;
        }

        if (k == ASCII_Q) {
            break;
        }

        printf("%d\n", k);
    }
}
