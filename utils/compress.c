/* compress.c
 *
 * Copyright 2025 Anivice Ives
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>
#include <fcntl.h>      // For open
#include <unistd.h>     // For read, write, close
#include <sys/stat.h>   // For fstat

// written in C since it's going to be way faster
// both compiling and executing

unsigned char* compress_data(const unsigned char* src, uint64_t len, uint64_t* compressed_data_len);

// Helper function to read entire file into memory using Linux system calls
unsigned char* read_file(const char* filename, uint64_t* file_size)
{
    if (filename == NULL || file_size == NULL) {
        return nullptr;
    }

    const int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return nullptr;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return nullptr;
    }

    *file_size = st.st_size;
    unsigned char* buffer = (unsigned char*)malloc(*file_size);
    if (buffer == NULL) {
        perror("malloc");
        close(fd);
        return nullptr;
    }

    ssize_t bytes_read = 0;
    size_t total_read = 0;
    while (total_read < *file_size) {
        bytes_read = read(fd, buffer + total_read, *file_size - total_read);
        if (bytes_read < 0) {
            perror("read");
            free(buffer);
            close(fd);
            return nullptr;
        }
        if (bytes_read == 0) {
            break; // EOF
        }
        total_read += bytes_read;
    }

    close(fd);

    if (total_read != *file_size) {
        fprintf(stderr, "Incomplete read: expected %llu bytes, got %zu bytes\n",
                (unsigned long long)(*file_size), total_read);
        free(buffer);
        return nullptr;
    }

    return buffer;
}

// Helper function to write data to a file using Linux system calls
int write_file(const char* filename, const unsigned char* data, const uint64_t len)
{
    if (filename == NULL || data == NULL) {
        return -1;
    }

    const int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    ssize_t bytes_written = 0;
    size_t total_written = 0;
    while (total_written < len) {
        bytes_written = write(fd, data + total_written, len - total_written);
        if (bytes_written < 0) {
            perror("write");
            close(fd);
            return -1;
        }
        total_written += bytes_written;
    }

    close(fd);
    return 0;
}

// Main function: Compress data from argv[1] to argv[2]
int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* source_file = argv[1];
    const char* dest_file = argv[2];

    // Read source file into memory
    uint64_t src_size = 0;
    unsigned char* src_data = read_file(source_file, &src_size);
    if (src_data == NULL) {
        fprintf(stderr, "Failed to read source file: %s\n", source_file);
        return EXIT_FAILURE;
    }

    // Compress the data
    uint64_t compressed_size = 0;
    unsigned char* compressed_data = compress_data(src_data, src_size, &compressed_size);
    free(src_data); // Free the source data as it's no longer needed

    if (compressed_data == NULL) {
        fprintf(stderr, "Compression failed.\n");
        return EXIT_FAILURE;
    }

    // Write compressed data to destination file
    if (write_file(dest_file, compressed_data, compressed_size) != 0) {
        fprintf(stderr, "Failed to write compressed data to: %s\n", dest_file);
        free(compressed_data);
        return EXIT_FAILURE;
    }

    free(compressed_data); // Free the compressed data

    printf("Compression successful.\n");
    printf("Original size: %llu bytes\n", (unsigned long long)src_size);
    printf("Compressed size: %llu bytes\n", (unsigned long long)compressed_size);

    return EXIT_SUCCESS;
}
