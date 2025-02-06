// mutex.c
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

    // Open (or create) the file that will act as our mutex container.
    int fd = open(mutex_file, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error creating mutex file");
        return EXIT_FAILURE;
    }

    printf("Mutex file '%s' created (or already exists).\n", mutex_file);
    close(fd);
    return EXIT_SUCCESS;
}
