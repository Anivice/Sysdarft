/* ASCIIKeymap.h
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

#ifndef KEYMAP_H
#define KEYMAP_H

enum KeyCode : short {
    NO_KEY = -1,
    // ASCII control characters (0–31)
    ASCII_NUL = 0,
    ASCII_SOH = 1,
    ASCII_STX = 2,
    ASCII_ETX = 3,
    ASCII_EOT = 4,
    ASCII_ENQ = 5,
    ASCII_ACK = 6,
    ASCII_BEL = 7,
    ASCII_BS  = 8,
    ASCII_HT  = 9,
    ASCII_LF  = 10,
    ASCII_VT  = 11,
    ASCII_FF  = 12,
    ASCII_CR  = 13,
    ASCII_SO  = 14,
    ASCII_SI  = 15,
    ASCII_DLE = 16,
    ASCII_DC1 = 17,
    ASCII_DC2 = 18,
    ASCII_DC3 = 19,
    ASCII_DC4 = 20,
    ASCII_NAK = 21,
    ASCII_SYN = 22,
    ASCII_ETB = 23,
    ASCII_CAN = 24,
    ASCII_EM  = 25,
    ASCII_SUB = 26,
    ASCII_ESC = 27,
    ASCII_FS  = 28,
    ASCII_GS  = 29,
    ASCII_RS  = 30,
    ASCII_US  = 31,

    // Printable ASCII characters (32–126)
    ASCII_SPACE          = 32,
    ASCII_EXCLAMATION    = 33,
    ASCII_DOUBLE_QUOTE   = 34,
    ASCII_HASH           = 35,
    ASCII_DOLLAR         = 36,
    ASCII_PERCENT        = 37,
    ASCII_AMPERSAND      = 38,
    ASCII_SINGLE_QUOTE   = 39,
    ASCII_LEFT_PAREN     = 40,
    ASCII_RIGHT_PAREN    = 41,
    ASCII_ASTERISK       = 42,
    ASCII_PLUS           = 43,
    ASCII_COMMA          = 44,
    ASCII_MINUS          = 45,
    ASCII_PERIOD         = 46,
    ASCII_SLASH          = 47,
    ASCII_0              = 48,
    ASCII_1              = 49,
    ASCII_2              = 50,
    ASCII_3              = 51,
    ASCII_4              = 52,
    ASCII_5              = 53,
    ASCII_6              = 54,
    ASCII_7              = 55,
    ASCII_8              = 56,
    ASCII_9              = 57,
    ASCII_COLON          = 58,
    ASCII_SEMICOLON      = 59,
    ASCII_LESS_THAN      = 60,
    ASCII_EQUAL          = 61,
    ASCII_GREATER_THAN   = 62,
    ASCII_QUESTION       = 63,
    ASCII_AT             = 64,
    ASCII_A              = 65,
    ASCII_B              = 66,
    ASCII_C              = 67,
    ASCII_D              = 68,
    ASCII_E              = 69,
    ASCII_F              = 70,
    ASCII_G              = 71,
    ASCII_H              = 72,
    ASCII_I              = 73,
    ASCII_J              = 74,
    ASCII_K              = 75,
    ASCII_L              = 76,
    ASCII_M              = 77,
    ASCII_N              = 78,
    ASCII_O              = 79,
    ASCII_P              = 80,
    ASCII_Q              = 81,
    ASCII_R              = 82,
    ASCII_S              = 83,
    ASCII_T              = 84,
    ASCII_U              = 85,
    ASCII_V              = 86,
    ASCII_W              = 87,
    ASCII_X              = 88,
    ASCII_Y              = 89,
    ASCII_Z              = 90,
    ASCII_LEFT_BRACKET   = 91,
    ASCII_BACKSLASH      = 92,
    ASCII_RIGHT_BRACKET  = 93,
    ASCII_CARET          = 94,
    ASCII_UNDERSCORE     = 95,
    ASCII_BACKTICK       = 96,
    ASCII_a              = 97,
    ASCII_b              = 98,
    ASCII_c              = 99,
    ASCII_d              = 100,
    ASCII_e              = 101,
    ASCII_f              = 102,
    ASCII_g              = 103,
    ASCII_h              = 104,
    ASCII_i              = 105,
    ASCII_j              = 106,
    ASCII_k              = 107,
    ASCII_l              = 108,
    ASCII_m              = 109,
    ASCII_n              = 110,
    ASCII_o              = 111,
    ASCII_p              = 112,
    ASCII_q              = 113,
    ASCII_r              = 114,
    ASCII_s              = 115,
    ASCII_t              = 116,
    ASCII_u              = 117,
    ASCII_v              = 118,
    ASCII_w              = 119,
    ASCII_x              = 120,
    ASCII_y              = 121,
    ASCII_z              = 122,
    ASCII_LEFT_BRACE     = 123,
    ASCII_PIPE           = 124,
    ASCII_RIGHT_BRACE    = 125,
    ASCII_TILDE          = 126,
    ASCII_DEL            = 127,
};

#endif // KEYMAP_H
