#ifndef SYSDARFT_SPEAKER_H
#define SYSDARFT_SPEAKER_H

#include <vector>
#include <pulse/simple.h>
#include <pulse/error.h>

class PCMPlayer
{

};

std::vector<char> extractPcmDataFromWav(const unsigned char* wavData, size_t dataSize);

#endif //SYSDARFT_SPEAKER_H
