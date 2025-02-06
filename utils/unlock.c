// unlock.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mutex_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *mutex_file = argv[1];

    // Remove the mutex file to unlock.
    if (unlink(mutex_file) == -1) {
        perror("Error unlocking mutex (removing file)");
        return EXIT_FAILURE;
    }

    printf("Mutex unlocked successfully. The lock file '%s' has been removed.\n", mutex_file);
    return EXIT_SUCCESS;
}
