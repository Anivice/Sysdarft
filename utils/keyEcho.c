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
        usleep(100);
        const int k = read_keyControl();
        if (k == NO_KEY) {
            continue;
        }

        printf("%d\n", k);
    }
}
