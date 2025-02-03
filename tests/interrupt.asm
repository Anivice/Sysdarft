; interrupt.asm
;
; Copyright 2025 Anivice Ives
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.
;
; SPDX-License-Identifier: GPL-3.0-or-later
;

%ifndef _INTERRUPT_ASM_
%define _INTERRUPT_ASM_

.equ 'REFRESH', 'int < $8(0x18) >'
.equ 'SETCUSP', 'int < $8(0x11) >'
.equ 'INTGETC', 'int < $8(0x14) >'

%define KBFLUSH int < $8(0x19) >

%endif ; _INTERRUPT_ASM_
