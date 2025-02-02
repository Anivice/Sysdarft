#!/usr/bin/env bash

string="$1"
resource_header="$2"
IFS=' ' read -r -a LIST <<< "$string"

rm -rf "$resource_header"

{
    echo "/* Resource.h
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
 */"

    echo "#ifndef _RESOURCE_H_"
    echo "#define _RESOURCE_H_"
    echo

    for item in "${LIST[@]}"; do
        {
            echo "extern unsigned char $item [];"
            echo "extern unsigned int ""$item""_len;"
            echo "extern unsigned long long int ""$item""_original_len;"
        }
    done

    # font list string table
    echo "inline const char * font_list_string_table [] = { "

    for item in "${LIST[@]}"; do
        if echo "$item" 2> /dev/null| grep font 2> /dev/null > /dev/null; then
            echo "    \"$item\","
        fi
    done

    echo -e "};\n "

    # font list reference table
    echo "inline const unsigned char * font_list_reference_table [] = { "

    for item in "${LIST[@]}"; do
        if echo "$item" 2> /dev/null| grep font 2> /dev/null > /dev/null; then
            echo "    $item,"
        fi
    done

    echo -e "};\n "

    # font list compressed length table
    echo "inline const unsigned int font_list_compressed_length_table [] = { "

    for item in "${LIST[@]}"; do
        if echo "$item" 2> /dev/null| grep font 2> /dev/null > /dev/null; then
            echo "    ""$item""_len,"
        fi
    done

    echo -e "};\n "

    # font list original length table
    echo "inline const unsigned long long int font_list_original_length_table [] = { "

    for item in "${LIST[@]}"; do
        if echo "$item" 2> /dev/null| grep font 2> /dev/null > /dev/null; then
            echo "    ""$item""_original_len,"
        fi
    done

    echo -e "};\n "

    echo "#endif // _RESOURCE_H_"
} >> "$resource_header"
