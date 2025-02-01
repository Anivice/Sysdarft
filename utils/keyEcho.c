// ExampleUsage.cpp
#include <ASCIIKeymap.h>
#include <stdio.h>

extern enum KeyCode read_keyControl();

int main()
{
    while (true)
    {
        const int k = read_keyControl();
        if (k == NO_KEY) {
            continue;
        }

        printf("%d\n", k);
    }

    return 0;
}
