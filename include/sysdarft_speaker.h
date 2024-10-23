#ifndef SYSDARFT_SPEAKER_H
#define SYSDARFT_SPEAKER_H

#include <pulse/error.h>
#include <pulse/simple.h>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

struct pcm_data_t {
    const unsigned char * data;
    size_t data_len;
};

class pulseaudio_connection_t
{
private:
    std::atomic < pa_simple * > sample = nullptr;
    std::mutex lock;

public:
    void initialize(const std::string & comm_name = "Sysdarft Pulse Audio Connection",
        const std::string & stream_name = "Playback Stream");
    void append_buffer(const std::vector < char > &);
    void cleanup();
};

pcm_data_t extractPcmDataFromWav(const unsigned char* wavData, size_t dataSize);

#endif //SYSDARFT_SPEAKER_H
