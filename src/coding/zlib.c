/* zlib.c
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
#include <string.h>
#include <zlib.h>

// Function to compress data
unsigned char* compress_data(const unsigned char* src, const uint64_t len, uint64_t* compressed_data_len)
{
    if (src == NULL || compressed_data_len == NULL) {
        return nullptr;
    }

    // Initialize z_stream
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    // Initialize the deflate process
    if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
        return nullptr;
    }

    // Estimate the upper bound of compressed data size
    uLong bound = compressBound(len);
    unsigned char* out = (unsigned char*)malloc(bound);
    if (out == NULL) {
        deflateEnd(&strm);
        return nullptr;
    }

    strm.next_in = (Bytef*)src;
    strm.avail_in = (uInt)len;
    strm.next_out = out;
    strm.avail_out = bound;

    // Perform compression
    int ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        // Compression did not finish successfully
        deflateEnd(&strm);
        free(out);
        return nullptr;
    }

    *compressed_data_len = strm.total_out;

    // Clean up
    deflateEnd(&strm);

    // Optionally, shrink the buffer to the actual size
    unsigned char* resized_out = (unsigned char*)realloc(out, *compressed_data_len);
    if (resized_out == NULL) {
        // If realloc fails, keep the original buffer
        return out;
    }

    return resized_out;
}

// Function to decompress data
unsigned char* decompress_data(const unsigned char* src, const uint64_t len, uint64_t* decompressed_data_len)
{
    if (src == NULL || decompressed_data_len == NULL) {
        return nullptr;
    }

    // Initialize z_stream
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    // Initialize the inflate process
    if (inflateInit(&strm) != Z_OK) {
        return nullptr;
    }

    // Allocate initial buffer for decompressed data
    size_t out_size = len * 4; // Initial guess; adjust as needed
    unsigned char* out = (unsigned char*)malloc(out_size);
    if (out == NULL) {
        inflateEnd(&strm);
        return nullptr;
    }

    strm.next_in = (Bytef*)src;
    strm.avail_in = (uInt)len;
    strm.next_out = out;
    strm.avail_out = (uInt)out_size;

    // Inflate loop
    while (1) {
        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            break; // Finished successfully
        }
        if (ret != Z_OK) {
            // Decompression error
            inflateEnd(&strm);
            free(out);
            return nullptr;
        }

        if (strm.avail_out == 0) {
            // Need to allocate more space
            size_t old_size = out_size;
            out_size *= 2;
            unsigned char* temp = (unsigned char*)realloc(out, out_size);
            if (temp == NULL) {
                inflateEnd(&strm);
                free(out);
                return nullptr;
            }
            out = temp;
            strm.next_out = out + old_size;
            strm.avail_out = (uInt)(out_size - old_size);
        }
    }

    *decompressed_data_len = strm.total_out;

    // Clean up
    inflateEnd(&strm);

    // Optionally, shrink the buffer to the actual size
    unsigned char* resized_out = (unsigned char*)realloc(out, *decompressed_data_len);
    if (resized_out == NULL) {
        // If realloc fails, keep the original buffer
        return out;
    }

    return resized_out;
}
