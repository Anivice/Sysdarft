#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <string>
#include <thread>
#include <SysdarftDebug.h>
#include <SysdarftMemory.h>

extern unsigned char bell_wav[];
extern unsigned int bell_wav_len;
extern std::mutex bell_memory_access_mutex;

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI : public SysdarftCPUMemoryAccess
{
public:
    explicit SysdarftCursesUI(uint64_t memory);
    ~SysdarftCursesUI() override;
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

    void recalc_offsets();
    void render_screen();

private:
    int offset_x;
    int offset_y;
    char * video_memory;
    bool is_inited = false;
    int vsb;
    std::atomic < bool > running;
    std::vector<std::thread> sound_thread_pool;

    char& video_at(const int x, const int y) {
        return video_memory[y * V_WIDTH + x];
    }

    static void play_bell_sound(const std::atomic < bool > & running_flag);
};

#endif // UI_CURSES_H
