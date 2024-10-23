#include <pulse/simple.h>
#include <pulse/error.h>
#include <cstdio>
#include <cstdlib>
#include <res_packer.h>
#include <sysdarft_speaker.h>

// Example PCM parameters: 16-bit signed, little-endian, 44.1kHz, mono
#define SAMPLE_RATE 44100
#define SAMPLE_FORMAT PA_SAMPLE_S16LE // 16-bit signed little-endian
#define CHANNELS 1

int main()
{
    initialize_resource_filesystem();

    auto file = get_res_file_list().at("scripts_res_sound_hdd_failure.wav");

    // PulseAudio parameters
    pa_sample_spec ss;
    ss.format = SAMPLE_FORMAT;
    ss.rate = SAMPLE_RATE;
    ss.channels = CHANNELS;

    // Initialize PulseAudio connection
    int error;
    pa_simple *s = pa_simple_new(nullptr, "PCM Player", PA_STREAM_PLAYBACK, nullptr, "playback", &ss, nullptr, nullptr, &error);
    if (!s) {
        fprintf(stderr, "PulseAudio connection failed: %s\n", pa_strerror(error));
        return 1;
    }

    // Write PCM data to PulseAudio
    if (pa_simple_write(s, file.file_content + 44, file.file_length - 44, &error) < 0)
    {
        fprintf(stderr, "Failed to play PCM data: %s\n", pa_strerror(error));
        pa_simple_free(s);
        return 1;
    }

    // Ensure all data is played
    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, "Failed to drain: %s\n", pa_strerror(error));
        pa_simple_free(s);
        return 1;
    }

    // Cleanup
    pa_simple_free(s);
    printf("PCM data played successfully.\n");
    return 0;
}
