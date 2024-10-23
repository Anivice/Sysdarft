#include <res_packer.h>
#include <sysdarft_speaker.h>
#include <thread>

int main()
{
    initialize_resource_filesystem();
    auto file = get_res_file_list().at("scripts_res_sound_hdd_failure.wav");
    auto file2 = get_res_file_list().at("scripts_res_sound_hdd_noise.wav");
    auto pcm_data = extractPcmDataFromWav(file.file_content, file.file_length);
    auto pcm_data2 = extractPcmDataFromWav(file2.file_content, file2.file_length);
    std::vector<char> vector_data(pcm_data.data, pcm_data.data + pcm_data.data_len);
    std::vector<char> vector_data2(pcm_data2.data, pcm_data2.data + pcm_data2.data_len);

    auto audio = [&](pulseaudio_connection_t * player, const std::vector<char> & buffer) {
        player->initialize();
        player->append_buffer(buffer);
        player->cleanup();
    };
    pulseaudio_connection_t player1, player2;
    std::thread Thread(audio, &player1, vector_data),
                Thread2(audio, &player2, vector_data2);
    Thread.detach();
    Thread2.detach();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    player1.cleanup();
    player2.cleanup();
    printf("PCM data played successfully.\n");
    return 0;
}
