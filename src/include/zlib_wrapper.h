#ifndef ZLIB_WRAPPER_H
#define ZLIB_WRAPPER_H

#include <cstdint>

extern "C" unsigned char* decompress_data(const unsigned char* src, uint64_t len, uint64_t* decompressed_data_len);

#endif //ZLIB_WRAPPER_H
