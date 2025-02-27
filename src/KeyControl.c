/* KeyControl.c
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

#include <ASCIIKeymap.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

// Flags to ensure one-time initialization.
static int nonblocking_set = 0;
static int raw_mode_set = 0;
static struct termios orig_termios;

/**
 * @brief Enables raw mode for STDIN.
 *
 * This disables canonical mode, echo, and signal generation so that control keys
 * (e.g., Ctrl+C, Ctrl+Z) are read as input rather than triggering signals.
 */
static void enableRawMode()
{
    if (raw_mode_set)
        return;

    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    raw = orig_termios;
    // Disable canonical mode, echo, and signals.
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);

    // **Re-enable CR to NL translation:**
    raw.c_iflag |= ICRNL;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    raw_mode_set = 1;
}

/**
 * @brief Reads one key from STDIN in non-blocking raw mode.
 *
 * If no key is available, returns NO_KEY.
 *
 * @return KeyCode corresponding to the input character or NO_KEY if no input.
 */
enum KeyCode read_keyControl()
{
    // Enable raw mode so control keys are delivered as characters.
    enableRawMode();

    // Set nonblocking mode on STDIN on the first call.
    if (!nonblocking_set)
    {
        const int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags == -1)
            return NO_KEY;
        if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1)
            return NO_KEY;
        nonblocking_set = 1;
    }

    unsigned char ch;
    const ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return NO_KEY;
        return NO_KEY;
    } else if (n == 0) {
        return NO_KEY;
    } else {
        return (enum KeyCode) ch;
    }
}
