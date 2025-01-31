/* SysdarftCursesUI.cpp
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

#include <cstring>
#include <thread>
#include <SFML/Audio.hpp>
#include <ncurses.h>
#include <SysdarftCursesUI.h>

std::mutex bell_memory_access_mutex;
std::vector < unsigned char > bell_sound_data_uncompressed;
extern "C" unsigned char* decompress_data(const unsigned char* src, uint64_t len, uint64_t* decompressed_data_len);

SysdarftCursesUI::SysdarftCursesUI(const uint64_t memory)
    :   SysdarftCPUMemoryAccess(memory),
        cursor_x(0), cursor_y(0),
        offset_x(0), offset_y(0),
        vsb(1)
{
    video_memory = (char*)SysdarftCPUMemoryAccess::Memory[184].data();
    // Initialize video memory with spaces
    for (int i = 0; i < V_HEIGHT * V_WIDTH; i++) {
        video_memory[i] = ' ';
    }

    log("Decompressing sound file...\n");

    // uncompress sound
    std::lock_guard<std::mutex> lock(bell_memory_access_mutex);
    uint64_t bell_sound_data_len = 0;
    const auto data = decompress_data(bell_sound, bell_sound_len, &bell_sound_data_len);
    if (bell_sound_data_len != bell_sound_original_len) {
        throw SysdarftBaseError("Sound file corrupted");
    }

    bell_sound_data_uncompressed.resize(bell_sound_data_len);
    std::memcpy(bell_sound_data_uncompressed.data(), data, bell_sound_data_len);
    free(data);

    log("Sound file decompressed!\n");
}

SysdarftCursesUI::~SysdarftCursesUI()
{
    running = false;
    for (auto & thread : sound_thread_pool)
    {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void SysdarftCursesUI::initialize()
{
    if (is_inited) {
        return;
    }
    is_inited = true;

    initscr();            // Start curses mode
    cbreak();             // Disable line buffering
    noecho();             // Don't echo typed characters
    keypad(stdscr, TRUE); // Enable special keys

    recalc_offsets();
    clear();
    curs_set(1);
    render_screen();      // Render initial screen
    vsb = 1;
}

void SysdarftCursesUI::start_again()
{
    if (is_inited) {
        return;
    }
    is_inited = true;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    recalc_offsets();
    clear();
    curs_set(1);         // Ensure cursor visibility is reset
    render_screen();
}

void SysdarftCursesUI::ringbell()
{
    running = true;
    sound_thread_pool.emplace_back(play_bell_sound, std::ref(running));
}

void SysdarftCursesUI::cleanup()
{
    if (is_inited) {
        endwin();
        is_inited = false;
    }
}

void SysdarftCursesUI::set_cursor(const int x, const int y)
{
    // Clamp within virtual screen bounds [0,79]x[0,24]
    cursor_x = std::clamp(x, 0, V_WIDTH  - 1);
    cursor_y = std::clamp(y, 0, V_HEIGHT - 1);

    // if curses mode is disabled, skip
    if (!is_inited) {
        return;
    }

    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();
}

void SysdarftCursesUI::set_cursor_visibility(const bool visible)
{
    vsb = visible;
}

void SysdarftCursesUI::teletype(const char text)
{
    if (!is_inited) {
        std::cout << text;
        return;
    }

    int current_x = cursor_x;
    int current_y = cursor_y;

    // Store character in video memory
    video_at(current_x, current_y) = text;
    current_x++;

    // Update cursor position after printing
    if (current_x >= V_WIDTH) {
        newline();
    } else {
        set_cursor(current_x, current_y);
    }

    render_screen();
}

void SysdarftCursesUI::newline()
{
    if (!is_inited) {
        std::cout << std::endl;
        return;
    }

    if (cursor_y == V_HEIGHT - 1)
    {
        for (uint64_t i = 0; i < V_HEIGHT - 1; i++) {
            std::memcpy(video_memory + i * V_WIDTH,
                video_memory + (i + 1) * V_WIDTH, V_WIDTH);
        }

        std::memset(video_memory + (V_HEIGHT - 1) * V_WIDTH, ' ', V_WIDTH);
        cursor_x = 0;
        set_cursor(cursor_x, cursor_y);
        render_screen();
    }
    else
    {
        cursor_x = 0;
        cursor_y++;
        set_cursor(cursor_x, cursor_y);
    }
}

void SysdarftCursesUI::handle_resize()
{
    if (!is_inited) {
        return;
    }

    recalc_offsets();
    render_screen();
    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();
}

void SysdarftCursesUI::recalc_offsets()
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // Horizontal centering or clamping
    if (cols >= V_WIDTH) {
        offset_x = (cols - V_WIDTH) / 2;
    } else {
        offset_x = 0;
    }

    // Vertical centering or clamping
    if (rows >= V_HEIGHT) {
        offset_y = (rows - V_HEIGHT) / 2;
    } else {
        offset_y = 0;
    }
}

void SysdarftCursesUI::render_screen()
{
    // skip screen rendering when curses is disabled
    if (!is_inited) {
        return;
    }

    set_cursor_visibility(false);

    clear();
    // Render video_memory to the screen using current offsets
    for (int y = 0; y < V_HEIGHT; ++y) {
        for (int x = 0; x < V_WIDTH; ++x) {
            mvaddch(offset_y + y, offset_x + x, video_at(x, y));
        }
    }
    // Position the cursor in the re-rendered screen
    move(offset_y + cursor_y, offset_x + cursor_x);
    refresh();

    set_cursor_visibility(vsb);
}

void SysdarftCursesUI::play_bell_sound(const std::atomic < bool > & running_flag)
{
    sf::SoundBuffer buffer;
    {
        std::lock_guard<std::mutex> guard(bell_memory_access_mutex);
        if (!buffer.loadFromMemory(bell_sound_data_uncompressed.data(),
                bell_sound_data_uncompressed.size()))
        {
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
}
