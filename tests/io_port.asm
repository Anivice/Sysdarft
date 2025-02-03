; io_port.asm
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

%ifndef _IO_PORT_ASM_
%define _IO_PORT_ASM_

.equ 'DISK_SIZE',           '< $64(0x136) >'
.equ 'DISK_START_SEC',      '< $64(0x137) >'
.equ 'DISK_OPS_SEC_CNT',    '< $64(0x138) >'
.equ 'DISK_INPUT',          '< $64(0x139) >'

.equ 'FDA_SIZE',            '< $64(0x116) >'
.equ 'FDA_START_SEC',       '< $64(0x117) >'
.equ 'FDA_OPS_SEC_CNT',     '< $64(0x118) >'
.equ 'FDA_OUTPUT',          '< $64(0x11A) >'

; floppy disk B

%define FDB_SIZE            0x126
%define FDB_START_SEC       0x127
%define FDB_OPS_SEC_CONT    0x128
%define FDB_INPUT           0x129
%define FDB_OUTPUT          0x12A

.equ 'RTC_TIME',    '0x70'
.equ 'RTC_INT',     '0x71'

%endif ; _IO_PORT_ASM_
