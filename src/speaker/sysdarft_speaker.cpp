#include <sysdarft_speaker.h>
#include <debug.h>
#include <thread>

pcm_data_t extractPcmDataFromWav(const unsigned char* wavData, size_t dataSize)
{
    const size_t wavHeaderSize = 44;  // Standard PCM WAV header is 44 bytes

    // Ensure the data is large enough to contain at least the header
    if (dataSize <= wavHeaderSize) {
        throw sysdarft_error_t(sysdarft_error_t::INVALID_WAV_FILE);
    }

    // Skip the WAV header and extract the raw PCM data
    const unsigned char* pcmDataStart = wavData + wavHeaderSize;
    size_t pcmDataSize = dataSize - wavHeaderSize;

    return (pcm_data_t){.data = pcmDataStart, .data_len = pcmDataSize };
}

void pulseaudio_connection_t::initialize(const std::string & comm_name, const std::string & stream_name)
{
    if (sample) {
        return;
    }

    // PulseAudio parameters
    pa_sample_spec sample_space;
    sample_space.format = PA_SAMPLE_S16LE;
    sample_space.rate = 44100;
    sample_space.channels = 1;

    pa_buffer_attr buffer_attr;
    buffer_attr.maxlength = (uint32_t)-1;  // Max buffer length (use server's default)
    buffer_attr.tlength = (uint32_t)-1;    // Target length of the playback buffer
    buffer_attr.prebuf = (uint32_t)-1;     // Pre-buffering before playback starts
    buffer_attr.minreq = (uint32_t)-1;     // Minimum request size

    lock.lock();
    // Initialize PulseAudio connection
    int error;
    sample = pa_simple_new(nullptr,
        comm_name.c_str(),
        PA_STREAM_PLAYBACK,
        nullptr,
        stream_name.c_str(),
        &sample_space,
        nullptr,
        &buffer_attr,
        &error);
    lock.unlock();

    if (!sample) {
        throw sysdarft_error_t(sysdarft_error_t::PULSEAUDIO_CONNECTION_FAILED);
    }
}

void pulseaudio_connection_t::append_buffer(const std::vector < char > & data_buffer)
{
    // Calculate how many 128-byte chunks we need
    size_t chunk_size = 128;
    size_t num_chunks = (data_buffer.size() + chunk_size - 1) / chunk_size;

    // Resulting vector of arrays
    std::vector< char [128] > packed_data_buffer(num_chunks);

    // Fill the packed data buffer
    for (size_t i = 0; i < num_chunks; ++i)
    {
        // Determine how many bytes to copy for the last chunk
        size_t start_index = i * chunk_size;
        size_t copy_size = std::min(chunk_size, data_buffer.size() - start_index);

        // Copy the data from the flat buffer into the chunk
        std::memcpy(packed_data_buffer[i], data_buffer.data() + start_index, copy_size);
        if (copy_size < chunk_size) {
            std::memset(packed_data_buffer[i] + copy_size, 0, chunk_size - copy_size);
        }
    }

    for (const auto& chunk : packed_data_buffer)
    {
        lock.lock();
        int error = 0;

        // make sure it's not changed
        if (!sample) {
            return;
        }

        if (pa_simple_write(sample, chunk, 128, &error) < 0)
        {
            pa_simple_free(sample);
            sample = nullptr;
            throw sysdarft_error_t(sysdarft_error_t::PULSEAUDIO_BUFFER_APPEND_FAILED);
        }

        lock.unlock();
    }
}

void pulseaudio_connection_t::cleanup()
{
    lock.lock();

    if (!sample) {
        return;
    }

    int error;
    // Flush any remaining audio in the buffer
    if (pa_simple_flush(sample, &error) < 0) {
    }
    pa_simple_free(sample);
    sample = nullptr;

    lock.unlock();
}
