; typewriter.asm
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

.org 0xC1800

%include "./int_and_port.asm"

jmp                     <%cb>,                      <_start>

; _putc(%EXR0, linear position, %EXR1, ASCII Code)
_putc:
    pushall
    mov     .64bit      <%db>,                      <$(0xB8000)>
    push    .16bit      <%exr1>
    xor     .16bit      <%exr1>,                    <%exr1>
    xor     .32bit      <%her1>,                    <%her1>
    mov     .64bit      <%dp>,                      <%fer0>


    pop     .16bit      <%exr0>
    mov     .8bit       <*1&8(%db, %dp, $(0))>,     <%r0>

    REFRESH

    popall
    ret

; _newline(%EXR0, linear address)
_newline:
    pushall
    int                 <$(0x15)>
    div     .16bit      <$(80)>
    ; EXR0 quotient(row), EXR1 reminder(col)
    cmp     .16bit      <%exr0>,                    <$(24)>
    jbe                 <%cb>,                      <.scroll>

    xor     .16bit      <%exr1>,                    <%exr1>
    inc     .16bit      <%exr0>
    mul     .16bit      <$(80)>
    SETCUSP
    REFRESH
    jmp         <%cb>,      < .exit>

    .scroll:
        ; move content (scroll up)
        mov .64bit      <%db>,                      <$(0xB8000)>
        xor .64bit      <%dp>,                      <%dp>
        mov .64bit      <%eb>,                      <$(0xB8000 + 80)>
        xor .64bit      <%ep>,                      <%ep>
        mov .64bit      <%fer3>,                    <$(2000 - 80)>
        movs

        ; clear last line
        mov .64bit      <%fer3>,                    <$(80)>
        mov .64bit      <%eb>,                      <$(0xB8000)>
        mov .64bit      <%ep>,                      <$(2000 - 80)>
        xor .64bit      <%dp>,                      <%dp>
        .scroll.loop:
            mov .8bit   <*1&8(%eb, %ep, %dp)>,      <$(' ')>
            inc .64bit  <%dp>
            loop        <%cb>,                      <.scroll.loop>

        mov .16bit      <%exr0>,                    <$(2000 - 80)>
        SETCUSP
        REFRESH
    .exit:

    popall
    int                 <$(0x15)>
    ret

; _puts(%DB:%DP), null terminated string
_puts:
    pushall
    .loop:
        mov .8bit       <%r2>,                      <*1&8(%db, %dp, $(0))>

        cmp .8bit       <%r2>,                      <$(0)>
        je              <%cb>,                      <.exit>

        cmp .8bit       <%r2>,                      <$(0x0A)>
        jne             <%cb>,                      <.skip_newline>

        .newline:
        call            <%cb>,                      <_newline>
        mov .64bit      <%fer3>,                    <.last_offset>
        mov .16bit      <*1&16($(0), %fer3, $(0))>, <%exr0>
        jmp             <%cb>,      <.end>

        .skip_newline:
        xor .8bit       <%r3>,                      <%r3>
        mov .64bit      <%fer3>,                    <.last_offset>
        mov .16bit      <%exr0>,                    <*1&16($(0), %fer3, $(0))>
        call            <%cb>,                      <_putc>

        inc .16bit      <%exr0>
        cmp .16bit      <%exr0>,                    <$(2000)>
        je              <%cb>,                      <.newline>

        mov .16bit      <*1&16($(0), %fer3, $(0))>, <%exr0>
        SETCUSP

        .end:
        inc .64bit      <%dp>
        jmp             <%cb>,                      <.loop>

    .exit:
    popall
    ret

.last_offset:
    .16bit_data < 0 >

_start:
    mov .64bit          <%sb>,                                      <_stack_frame>
    mov .64bit          <%sp>,                                      <$(0xFFF)>

    mov .64bit          <*1&64($(0xA0000), $(16 * 5), $(8))>,       <_int_kb_abort>

    mov .64bit          <%dp>,                                      <.cache>
    xor .64bit          <%db>,                                      <%db>
    .loop:
        int             <$(0x14)>
        mov .16bit      <*1&16(%db, %dp, $(0))>,                    <%exr0>
        call            <%cb>,                                      <_puts>
        jmp             <%cb>,                                      <.loop>

    .cache:
    .64bit_data < 0 >

_int_kb_abort:
    int                 <$(0x17)>
    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.message>
    call                <%cb>,                                      <_puts>
    mov .64bit          <%fer3>,                                    <$(0x1FFFFF)>
    .wait:
    loop                <%cb>,                                      <.wait>
    xor .64bit          <%fer0>,                                    <%fer0>
    hlt
    .message:
    .string < "\nSystem shutdown!\n" >
    .8bit_data < 0 >

_stack_frame:
    .resvb < 0xFFF >
