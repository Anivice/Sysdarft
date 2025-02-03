; disk_io.asm
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
    mov     .64bit      <%db>,                      <$64(0xB8000)>
    push    .16bit      <%exr1>                                     
    xor     .16bit      <%exr1>,                    <%exr1>         
    xor     .32bit      <%her1>,                    <%her1>         
    mov     .64bit      <%dp>,                      <%fer0>         
                                                                    
                                                                    
    pop     .16bit      <%exr0>                                     
    mov     .8bit       <*1&8(%db, %dp, $8(0))>,    <%r0>

    REFRESH

    popall
    ret

; _newline(%EXR0, linear address)
_newline:
    pushall
    int                 <$8(0x15)>
    div     .16bit      <$16(80)>
    ; EXR0 quotient(row), EXR1 reminder(col)
    cmp     .16bit      <%exr0>,                    <$16(24)>
    jbe                 <%cb>,                      <.scroll>

    xor     .16bit      <%exr1>,                    <%exr1>
    inc     .16bit      <%exr0>
    mul     .16bit      <$16(80)>
    SETCUSP
    REFRESH
    jmp         <%cb>,      < .exit>

    .scroll:
        ; move content (scroll up)
        mov .64bit      <%db>,                      <$64(0xB8000)>
        xor .64bit      <%dp>,                      <%dp>
        mov .64bit      <%eb>,                      <$64(0xB8000 + 80)>
        xor .64bit      <%ep>,                      <%ep>
        mov .64bit      <%fer3>,                    <$64(2000 - 80)>
        movs

        ; clear last line
        mov .64bit      <%fer3>,                    <$64(80)>
        mov .64bit      <%eb>,                      <$64(0xB8000)>
        mov .64bit      <%ep>,                      <$64(2000 - 80)>
        xor .64bit      <%dp>,                      <%dp>
        .scroll.loop:
            mov .8bit   <*1&8(%eb, %ep, %dp)>,      <$8(' ')>
            inc .64bit  <%dp>
            loop        <%cb>,                      <.scroll.loop>

        mov .16bit      <%exr0>,                    <$16(2000 - 80)>
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
        mov .8bit       <%r2>,                      <*1&8(%db, %dp, $8(0))>

        cmp .8bit       <%r2>,                      <$8(0)>
        je              <%cb>,                      <.exit>

        cmp .8bit       <%r2>,                      <$8(0x0A)>
        jne             <%cb>,                      <.skip_newline>

        .newline:
        call            <%cb>,                      <_newline>
        mov .64bit      <%fer3>,                    <.last_offset>
        mov .16bit      <*1&16($8(0), %fer3, $8(0))>, <%exr0>
        jmp             <%cb>,      <.end>

        .skip_newline:
        xor .8bit       <%r3>,                      <%r3>
        mov .64bit      <%fer3>,                    <.last_offset>
        mov .16bit      <%exr0>,                    <*1&16($8(0), %fer3, $8(0))>
        call            <%cb>,                      <_putc>

        inc .16bit      <%exr0>
        cmp .16bit      <%exr0>,                    <$16(2000)>
        je              <%cb>,                      <.newline>

        mov .16bit      <*1&16($8(0), %fer3, $8(0))>, <%exr0>
        SETCUSP

        .end:
        inc .64bit      <%dp>
        jmp             <%cb>,                      <.loop>

    .exit:
    popall
    ret

.last_offset:
    .16bit_data < 0 >

; _print_num(%fer0)
_print_num:
    pushall

    xor .64bit          <%fer2>,                    <%fer2>       ; record occurrences of digits
    .loop:
        div .64bit      <$64(10)>
        ; %fer0 ==> ori
        ; %fer1 ==> reminder
        mov  .64bit     <%fer3>,                    <%fer1>
        add  .64bit     <%fer3>,                    <$64('0')>
        push .64bit     <%fer3>

        inc .64bit      <%fer2>

        cmp .64bit      <%fer0>,                    <$64(0x00)>
        jne             <%cb>,                      <.loop>

    xor .64bit          <%db>,                      <%db>
    mov .64bit          <%dp>,                      <.cache>

    mov .64bit          <%fer3>,                    <%fer2>
    .loop_pop:
        pop .64bit      <%fer0>
        mov .8bit       <*1&8(%db, %dp, $8(0))>,    <%r0>
        inc .64bit      <%dp>
        loop            <%cb>,                      <.loop_pop>

    mov .8bit           <*1&8(%db, %dp, $8(0))>,    <$8(0)>
    mov .64bit          <%dp>,                      <.cache>
    call                <%cb>,                      <_puts>

    popall
    ret

    .cache:
        .resvb < 16 >

; read disk to 0x0000:0x0000, length returned by %fer0
_reads:
    pushall
    in .64bit           DISK_SIZE,                  <%fer3>
    ; max 640 KB, meaning 1280 sectors
    cmp .64bit          <%fer3>,                    <$64(1280)>
    jl                  <%cb>,                      <.skip.trunc>

    mov .64bit          <%dp>,                      <.message.disk.too.big>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>
    mov .64bit          <%fer0>,                    <%fer3>
    call                <%cb>,                      <_print_num>
    mov .64bit          <%dp>,                      < .message.disk.too.big.tail >
    call                <%cb>,                      <_puts>

    mov .64bit          <%fer3>,                    <$64(1280)>

    mov .64bit          <%dp>,                      <.message.disk.resize>
    call                <%cb>,                      <_puts>
    mov .64bit          <%fer0>,                    <%fer3>
    call                <%cb>,                      <_print_num>
    mov .64bit          <%dp>,                      <.message.disk.resize.tail>
    call                <%cb>,                      <_puts>

    .skip.trunc:
    mov .64bit          <%dp>,                      <.message.disk.size>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    mov .64bit          <%fer0>,                    <%fer3>
    call                <%cb>,                      <_print_num>
    mov .64bit          <%dp>,                      <.message.sector>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    mov .64bit          <%dp>,                      <.message.reading>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    out .64bit          DISK_START_SEC,             <$64(0)>
    out .64bit          DISK_OPS_SEC_CNT,           <%fer3>
    mul .64bit          <$64(512)>
    mov .64bit          <%fer3>,                    <%fer0>
    xor .64bit          <%dp>,                      <%dp>
    xor .64bit          <%db>,                      <%db>
    ins .64bit          DISK_INPUT

    mov .64bit          <%fer0>,                    <.ret>
    mov .64bit          <*1&64(%fer0, $8(0), $8(0))>, <%fer3>

    mov .64bit          <%dp>,                      <.message.done>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    popall

    mov .64bit          <%fer0>,                    <.ret>
    mov .64bit          <%fer0>,                    <*1&64(%fer0, $8(0), $8(0))>

    ret

    .ret:
    .64bit_data < 0 >

    .message.disk.size:
    .string < "Detected disk has " >
    .8bit_data < 0 >

    .message.sector:
    .string < " sectors.\n" >
    .8bit_data < 0 >

    .message.reading:
    .string < "Reading disk..." >
    .8bit_data < 0 >
    .message.done:
    .string < "done.\n" >
    .8bit_data < 0 >

    .message.disk.too.big:
    .string < "Size of C: too big (" >
    .8bit_data < 0 >
    .message.disk.too.big.tail:
    .string < " sectors).\n" >
    .8bit_data < 0 >

    .message.disk.resize:
    .string < "Resized read length to " >
    .8bit_data < 0 >
    .message.disk.resize.tail:
    .string < " sectors.\n" >
    .8bit_data < 0 >

; _writes(%fer0)
_writes:
    pushall

    in .64bit           FDA_SIZE,                   <%fer3>
    push .64bit         <%fer0>

    ; %fer0 ==> %fer4 == .reads size
    mov .64bit          <%fer4>,                    <%fer0>

    ; %fer0 <== A: size
    mov .64bit          <%fer0>,                    <%fer3>
    mul .64bit          <$64(512)>

    ; compare %fer0 and %fer4
    cmp .64bit          <%fer0>,                    <%fer4>
    jbe                 <%cb>,                      <.skip.trunc>
    add .64bit          <%sp>,                      <$64(8)>
    push .64bit         <%fer0>

    mov .64bit          <%dp>,                      <.message.disk.too.small>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    mov .64bit          <%dp>,                      <.message.disk.resize>
    call                <%cb>,                      <_puts>
    call                <%cb>,                      <_print_num>
    mov .64bit          <%dp>,                      <.message.disk.resize.tail>
    call                <%cb>,                      <_puts>

    .skip.trunc:
    mov .64bit          <%dp>,                      <.message.disk.size>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    mov .64bit          <%fer0>,                    <%fer3>
    call                <%cb>,                      <_print_num>
    mov .64bit          <%dp>,                      <.message.sector>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    mov .64bit          <%dp>,                      <.message.writing>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    pop .64bit          <%fer0>
    push .64bit         <%fer0>
    div .64bit          <$64(512)>

    out .64bit          FDA_START_SEC,              <$64(0)>
    out .64bit          FDA_OPS_SEC_CNT,            <%fer0>
    pop .64bit          <%fer3>
    xor .64bit          <%dp>,                      <%dp>
    xor .64bit          <%db>,                      <%db>
    outs .64bit         FDA_OUTPUT

    popall
    ret

    .message.disk.size:
    .string < "Detected floppy A has " >
    .8bit_data < 0 >

    .message.sector:
    .string < " sectors.\n" >
    .8bit_data < 0 >

    .message.writing:
    .string < "Writing floppy disk...\n" >
    .8bit_data < 0 >
    .message.done:
    .string < "done.\n" >
    .8bit_data < 0 >

    .message.disk.too.small:
    .string < "Size of A: too small\n" >
    .8bit_data < 0 >

    .message.disk.resize:
    .string < "Resized write length to " >
    .8bit_data < 0 >
    .message.disk.resize.tail:
    .string < " bytes.\n" >
    .8bit_data < 0 >

_int_0x02_io_error:
    cmp .16bit          <%exr0>,                    <$16(0xF0)>
    je                  <%cb>,                      <.io_error>

    ; no such device
    mov .64bit          <%dp>,                      <.message.no.such.dev>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>
    KBFLUSH
    INTGETC
    mov .64bit          <%fer0>,                    <$64(6)>
    jmp                 <%cb>,                      <.error.type.end>

    .io_error:
    mov .64bit          <%dp>,                      <.message.io.error>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>
    KBFLUSH
    INTGETC
    mov .64bit          <%fer0>,                    <$64(5)>

    .error.type.end:
    hlt

    .message.io.error:
    .string < "IO ERROR!\nPress any key to shutdown..." >
    .8bit_data < 0 >

    .message.no.such.dev:
    .string < "Disk NOT present!\nPress any key to shutdown..." >
    .8bit_data < 0 >

_start:
    mov .64bit          <%sb>,                      <_stack_frame>
    mov .64bit          <%sp>,                      <$64(0xFFF)>

    ; install error handler
    mov .64bit          <*1&64($32(0xA0000), $16(0x02 * 16), $8(8))>, <_int_0x02_io_error>

    ; show welcome message
    mov .64bit          <%dp>,                      <.welcome>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    KBFLUSH
    INTGETC

    ; read from disk
    mov .64bit          <%dp>,                      <.reading_from_disk>
    call                <%cb>,                      <_puts>
    call                <%cb>,                      <_reads>
    push .64bit         <%fer0>

    mov .64bit          <%dp>,                      <.press_to_write_to_floppy>
    call                <%cb>,                      <_puts>

    KBFLUSH
    INTGETC

    mov .64bit          <%dp>,                      <.writint_to_floppy>
    call                <%cb>,                      <_puts>
    pop .64bit          <%fer0>
    call                <%cb>,                      <_writes>

    mov .64bit          <%dp>,                      <.exit.message>
    xor .64bit          <%db>,                      <%db>
    call                <%cb>,                      <_puts>

    KBFLUSH
    INTGETC
    xor .64bit          <%fer0>,                    <%fer0>
    hlt

.welcome:
    .string < "Hello!\n\nThis is Sysdarft Example A!\n\n\n" >
    .string < "Sysdarft is a hypothetical architecture that offers simplified instructions\n" >
    .string < "with potency for creating functional programs and even operating systems.\n" >
    .string < "By eliminating the need to maintain compatibility with historical designs,\n" >
    .string < "Sysdarft aims to be straightforward, avoiding complex details while maintaining\n" >
    .string < "consistency and functionality.\n\n\nPress any key to read from disk\n" >
    .8bit_data < 0 >

.reading_from_disk:
    .string < "Reading from disk...\n" >
    .8bit_data < 0 >

.press_to_write_to_floppy:
    .string < "Press any key to write to floppy disk A\n" >
    .8bit_data < 0 >

.writint_to_floppy:
    .string < "Writing to floppy disk A...\n" >
    .8bit_data < 0 >

.exit.message:
    .string < "Press any key to shutdown...\n" >
    .8bit_data < 0 >

_stack_frame:
    .resvb < 0xFFF >
