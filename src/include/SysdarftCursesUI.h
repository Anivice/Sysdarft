#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <ncurses.h>
#include <string>
#include <thread>
#include <SFML/Audio.hpp>
#include <SysdarftDebug.h>

extern unsigned char bell_wav[];
extern unsigned int bell_wav_len;
extern std::mutex bell_memory_access_mutex;

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI {
public:
    SysdarftCursesUI();
    ~SysdarftCursesUI();
    void initialize();
    void cleanup();
    void set_cursor(int x, int y);
    void set_cursor_visibility(bool visible);
    void teletype(char text);
    void newline();
    void handle_resize();
    void start_again();
    void ringbell();

protected:
    int cursor_x;
    int cursor_y;

private:
    int offset_x;
    int offset_y;
    char video_memory[V_HEIGHT][V_WIDTH]{};
    bool is_inited = false;
    int vsb;
    std::atomic < bool > running;

    void recalc_offsets();
    void render_screen();

    std::vector<std::thread> sound_thread_pool;
    static void play_bell_sound(const std::atomic < bool > & running_flag)
    {
        sf::SoundBuffer buffer;
        {
            std::lock_guard<std::mutex> guard(bell_memory_access_mutex);
            if (!buffer.loadFromMemory(bell_wav, bell_wav_len)) {
                log("Failed to load bell wav from memory");
                return;
            }
        }

        sf::Sound sound;
        sound.setBuffer(buffer);
        sound.play();

        // Wait until the sound finishes playing
        while (running_flag && sound.getStatus() == sf::Sound::Playing) {
            sf::sleep(sf::milliseconds(50));
        }
    };
};

#endif // UI_CURSES_H
