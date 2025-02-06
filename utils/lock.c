// lock.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mutex_filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *mutex_file = argv[1];

    // Attempt to create the file exclusively.
    int fd = open(mutex_file, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Error: Mutex is already locked (file '%s' exists).\n", mutex_file);
        } else {
            perror("Error locking mutex");
        }
        return EXIT_FAILURE;
    }

    printf("Mutex locked successfully. The lock file '%s' now exists.\n", mutex_file);
    close(fd);
    return EXIT_SUCCESS;
}
