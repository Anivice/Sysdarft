/* SysdarftCursesUI.h
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

#ifndef UI_CURSES_H
#define UI_CURSES_H

#include <ASCIIKeymap.h>
#include <SysdarftDebug.h>
#include <SysdarftMemory.h>
#include <TerminalDisplay.hpp>
#include <WorkerThread.h>
#include <string>
#include <thread>

extern std::vector < unsigned char > bell_sound_data_uncompressed;
extern std::mutex bell_memory_access_mutex;

extern "C" enum KeyCode read_keyControl();

// -----------------------------------------------------
// Virtual screen dimensions
// -----------------------------------------------------
static constexpr int V_WIDTH  = 80;
static constexpr int V_HEIGHT = 25;

class SYSDARFT_EXPORT_SYMBOL SysdarftCursesUI : public SysdarftCPUMemoryAccess
{
public:
    explicit SysdarftCursesUI(uint64_t memory, const std::string & font_name);
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
    // External halt is handled at upper level
    std::atomic<bool> SystemHalted = false; // TODO: Shutdown should be an interruption
    std::atomic < bool > KeyboardIntAbort = false;
    std::atomic < bool > CtrlZShutdownRequested = false;
    [[nodiscard]] bool get_is_inited() const { return is_inited; }

    int cursor_x;
    int cursor_y;

    void recalc_offsets();
    void render_screen();
    void launch_gui();
    TerminalDisplay GUIDisplay;

    void flush_input_buffer();
    int get_input();

private:
    int offset_x;
    int offset_y;
    char * video_memory;
    bool is_inited = false;
    int vsb;
    std::atomic < bool > running;
    std::vector<std::thread> sound_thread_pool;
    std::vector < int > captured_input;
    std::mutex input_mutex;

    void monitor_console_input(std::atomic < bool > & running);

    WorkerThread ConsoleInputThread;

    char& video_at(const int x, const int y) {
        std::lock_guard<std::mutex> lock(SysdarftCPUMemoryAccess::MemoryAccessMutex);
        return video_memory[y * V_WIDTH + x];
    }

    static void play_bell_sound(const std::atomic < bool > & running_flag);
    void gui_input_handler(int);
    std::array<char, V_HEIGHT * V_WIDTH> video_memory_puller();
};

#endif // UI_CURSES_H
