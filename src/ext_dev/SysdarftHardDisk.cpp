/* SysdarftHardDisk.cpp
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

#include <SysdarftDebug.h>
#include <SysdarftDisks.h>

std::streamoff getFileSize(std::fstream& file)
{
    // Save current position (if needed)
    const std::streampos currentPos = file.tellg();

    // Seek to the end to determine file size
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();

    // Restore the pointer to the original position (optional)
    file.seekg(currentPos);

    return size;
}
