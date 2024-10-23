#include <sysdarft_speaker.h>
#include <iostream>
#include <algorithm>

std::vector<char> extractPcmDataFromWav(const unsigned char* wavData, size_t dataSize)
{
    const size_t wavHeaderSize = 44;  // Standard PCM WAV header is 44 bytes

    // Ensure the data is large enough to contain at least the header
    if (dataSize <= wavHeaderSize) {
        throw std::runtime_error("Invalid WAV data size.");
    }

    // Skip the WAV header and extract the raw PCM data
    const unsigned char* pcmDataStart = wavData + wavHeaderSize;
    size_t pcmDataSize = dataSize - wavHeaderSize;

    // Copy the PCM data into a vector
    std::vector<char> pcmData(pcmDataStart, pcmDataStart + pcmDataSize);

    return pcmData;
}
