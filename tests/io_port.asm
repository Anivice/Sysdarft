%ifndef _IO_PORT_ASM_
%define _IO_PORT_ASM_

.equ 'DISK_SIZE',           '< $(0x136) >'
.equ 'DISK_START_SEC',      '< $(0x137) >'
.equ 'DISK_OPS_SEC_CNT',    '< $(0x138) >'
.equ 'DISK_INPUT',          '< $(0x139) >'

.equ 'FDA_SIZE',            '< $(0x116) >'
.equ 'FDA_START_SEC',       '< $(0x117) >'
.equ 'FDA_OPS_SEC_CNT',     '< $(0x118) >'
.equ 'FDA_OUTPUT',          '< $(0x11A) >'

; floppy disk B

%define FDB_SIZE            0x126
%define FDB_START_SEC       0x127
%define FDB_OPS_SEC_CONT    0x128
%define FDB_INPUT           0x129
%define FDB_OUTPUT          0x12A

%endif ; _IO_PORT_ASM_
