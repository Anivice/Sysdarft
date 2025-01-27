%ifndef _INTERRUPT_ASM_
%define _INTERRUPT_ASM_

.equ 'REFRESH', 'int < $(0x18) >'
.equ 'SETCUSP', 'int < $(0x11) >'
.equ 'INTGETC', 'int < $(0x14) >'

%endif ; _INTERRUPT_ASM_
