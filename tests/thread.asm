; thread.asm
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

jmp                     <%cb>,                                      <_start>

; _putc(%EXR0, linear position, %EXR1, ASCII Code)
_putc:
    pushall
    mov     .64bit      <%db>,                                      <$64(0xB8000)>
    push    .16bit      <%exr1>
    xor     .16bit      <%exr1>,                                    <%exr1>
    xor     .32bit      <%her1>,                                    <%her1>
    mov     .64bit      <%dp>,                                      <%fer0>


    pop     .16bit      <%exr0>
    mov     .8bit       <*1&8(%db, %dp, $8(0))>,                    <%r0>

    REFRESH

    popall
    ret

; _newline(%EXR0, linear address)
_newline:
    pushall
    int                 <$8(0x15)>
    div     .16bit      <$16(80)>
    ; EXR0 quotient(row), EXR1 reminder(col)
    cmp     .16bit      <%exr0>,                                    <$16(24)>
    jbe                 <%cb>,                                      <.scroll>

    xor     .16bit      <%exr1>,                                    <%exr1>
    inc     .16bit      <%exr0>
    mul     .16bit      <$16(80)>
    SETCUSP
    REFRESH
    jmp                 <%cb>,                                      <.exit>

    .scroll:
        ; move content (scroll up)
        mov .64bit      <%db>,                                      <$64(0xB8000)>
        xor .64bit      <%dp>,                                      <%dp>
        mov .64bit      <%eb>,                                      <$64(0xB8000 + 80)>
        xor .64bit      <%ep>,                                      <%ep>
        mov .64bit      <%fer3>,                                    <$64(2000 - 80)>
        movs

        ; clear last line
        mov .64bit      <%fer3>,                                    <$64(80)>
        mov .64bit      <%eb>,                                      <$64(0xB8000)>
        mov .64bit      <%ep>,                                      <$64(2000 - 80)>
        xor .64bit      <%dp>,                                      <%dp>
        .scroll.loop:
            mov .8bit   <*1&8(%eb, %ep, %dp)>,                      <$8(' ')>
            inc .64bit  <%dp>
            loop        <%cb>,                                      <.scroll.loop>

        mov .16bit      <%exr0>,                                    <$16(2000 - 80)>
        SETCUSP
        REFRESH
    .exit:

    popall
    int                 <$8(0x15)>
    ret

; _puts(%DB:%DP), null terminated string
_puts:
    pushall
    .loop:
        mov .8bit       <%r2>,                                      <*1&8(%db, %dp, $8(0))>

        cmp .8bit       <%r2>,                                      <$8(0)>
        je              <%cb>,                                      <.exit>

        cmp .8bit       <%r2>,                                      <$8(0x0A)>
        jne             <%cb>,                                      <.skip_newline>

        .newline:
        call            <%cb>,                                      <_newline>
        mov .64bit      <%fer3>,                                    <.last_offset>
        mov .16bit      <*1&16($8(0), %fer3, $8(0))>,               <%exr0>
        jmp             <%cb>,                                      <.end>

        .skip_newline:
        xor .8bit       <%r3>,                                      <%r3>
        mov .64bit      <%fer3>,                                    <.last_offset>
        mov .16bit      <%exr0>,                                    <*1&16($8(0), %fer3, $8(0))>
        call            <%cb>,                                      <_putc>

        inc .16bit      <%exr0>
        cmp .16bit      <%exr0>,                                    <$16(2000)>
        je              <%cb>,                                      <.newline>

        mov .16bit      <*1&16($8(0), %fer3, $8(0))>,               <%exr0>
        SETCUSP

        .end:
        inc .64bit      <%dp>
        jmp             <%cb>,                                      <.loop>

    .exit:
    popall
    ret

.last_offset:
    .16bit_data < 0 >

int_puts:
    call                <%cb>,                                      <_puts>
    iret

threadA:
    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.message>

    .loop:
        int             <$8(0x80)>
        jmp             <%cb>,                                      <.loop>

    .message:
        .string < "A" >
        .8bit_data < 0 >

threadB:
    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.message>

    .loop:
        int             <$8(0x80)>
        jmp             <%cb>,                                      <.loop>

    .message:
        .string < "B" >
        .8bit_data < 0 >

_rtc:
    mov .64bit          <%fer1>,                                    <.current_thread>
    xor .64bit          <%fer2>,                                    <%fer2>
    mov .8bit           <%r0>,                                      <*1&8(%fer1, %fer2, $8(0))>
    cmp .8bit           <%r0>,                                      <$8(0)>
    je                  <%cb>,                                      <.is_a>

    .is_b:
        ; Save B
        xor .64bit      <%db>,                                      <%db>
        mov .64bit      <%dp>,                                      <_reg_threadb>
        mov .64bit      <%eb>,                                      <%sb>
        mov .64bit      <%ep>,                                      <%sp>
        mov .64bit      <%fer3>,                                    <$64(256)>
        movs

        ; Restore A
        mov .64bit      <%db>,                                      <%sb>
        mov .64bit      <%dp>,                                      <%sp>
        mov .64bit      <%ep>,                                      <_reg_threada>
        xor .64bit      <%eb>,                                      <%eb>
        mov .64bit      <%fer3>,                                    <$64(256)>
        movs

        xor .8bit       <%r0>,                                      <%r0>
        jmp             <%cb>,                                      <.end_switch>

    .is_a:
        ; we need to determine if A is being executed before save
        mov .64bit      <%fer4>,                                    <.a_started>
        mov .8bit       <%r1>,                                      <*1&8(%fer4, $8(0), $8(0))>
        cmp .8bit       <%r1>,                                      <$8(0)>
        jne             <%cb>,                                      <.save_a>

        mov .8bit       <*1&8(%fer4, $8(0), $8(0))>,                <$8(1)>
        jmp             <%cb>,                                      <.restore_b>

        ; Save A
        .save_a:
        xor .64bit      <%db>,                                      <%db>
        mov .64bit      <%dp>,                                      <_reg_threada>
        mov .64bit      <%eb>,                                      <%sb>
        mov .64bit      <%ep>,                                      <%sp>
        mov .64bit      <%fer3>,                                    <$64(256)>
        movs

        ; Restore B
        .restore_b:
        mov .64bit      <%db>,                                      <%sb>
        mov .64bit      <%dp>,                                      <%sp>
        mov .64bit      <%eb>,                                      <_reg_threadb>
        xor .64bit      <%ep>,                                      <%ep>
        mov .64bit      <%fer3>,                                    <$64(256)>
        movs

        mov .8bit       <%r0>,                                      <$8(1)>

    .end_switch:
    mov .8bit           <*1&8(%fer1, %fer2, $8(0))>,                <%r0>
    iret

    .current_thread:
        .8bit_data < 0 >
    .a_started:
        .8bit_data < 0 >

_shutdown:
    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.message>
    int                 <$8(0x13)>
    int                 <$8(0x13)>
    .loop:
        xor .16bit      <%exr0>,                                    <%exr0>
        mov .8bit       <%r0>,                                      <*1&8(%db, %dp, $8(0))>
        cmp .8bit       <%r0>,                                      <$8(0)>
        je              <%cb>,                                      <.end>
        int             <$8(0x10)>
        inc .64bit      <%dp>
        jmp             <%cb>,                                      <.loop>

    .end:
    hlt
    .message:
        .string < "System shutdown requested received!\n" >
        .8bit_data < 0 >

_start:
    mov .64bit          <%sb>,                                      <_stack_frame>
    mov .64bit          <%sp>,                                      <$64(0xFFF)>

    mov .64bit          <*1&64($32(0xA0000), $16(16 * 0x80), $8(8))>,   <int_puts>
    mov .64bit          <*1&64($32(0xA0000), $16(16 * 0x81), $8(8))>,   <_rtc>
    mov .64bit          <*1&64($32(0xA0000), $16(16 * 0x09), $8(8))>,   <_shutdown>
    mov .64bit          <*1&64($32(0xA0000), $16(16 * 0x05), $8(8))>,   <_shutdown>

    mov .64bit          <%dp>,                                          <_reg_threada>
    mov .64bit          <*1&64(%dp, $8(160), $8(0))>,                   <threadA>           ; IP
    mov .64bit          <*1&64(%dp, $8(136), $8(0))>,                   <_stack_threada>    ; SB
    mov .64bit          <*1&64(%dp, $8(144), $8(0))>,                   <$64(0xFFF)>        ; SP

    mov .64bit          <%dp>,                                          <_reg_threadb>
    mov .64bit          <*1&64(%dp, $8(160), $8(0))>,                   <threadB>           ; IP
    mov .64bit          <*1&64(%dp, $8(136), $8(0))>,                   <_stack_threadb>    ; SB
    mov .64bit          <*1&64(%dp, $8(144), $8(0))>,                   <$64(0xFFF)>        ; SP

    out .64bit          <$64(RTC_INT)>,                                 <$64(0x181)>        ; 0x81

    jmp                 <%cb>,                                          <$64(@)>

_reg_threada:
    .resvb < 256 >

_stack_threada:
    .resvb < 0xFFF >

_reg_threadb:
    .resvb < 256 >

_stack_threadb:
    .resvb < 0xFFF >

_stack_frame:
    .resvb < 0xFFF >
