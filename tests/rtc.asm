; rtc.asm
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


.equ 'RTC_TIME',    '0x70'
.equ 'RTC_INT',     '0x71'

%define SECONDS_IN_MINUTE   < $(60) >
%define SECONDS_IN_HOUR     < $(3600) >
%define SECONDS_IN_DAY      < $(86400) >

.org 0xC1800

%include "./interrupt.asm"

jmp                     <%cb>,                                      <_start>

; _putc(%EXR0, linear position, %EXR1, ASCII Code)
_putc:
    pushall
    mov     .64bit      <%db>,                                      <$(0xB8000)>
    push    .16bit      <%exr1>
    xor     .16bit      <%exr1>,                                    <%exr1>
    xor     .32bit      <%her1>,                                    <%her1>
    mov     .64bit      <%dp>,                                      <%fer0>

    pop     .16bit      <%exr0>
    mov     .8bit       <*1&8(%db, %dp, $(0))>,                     <%r0>

    REFRESH

    popall
    ret

; _newline(%EXR0, linear address)
_newline:
    pushall
    int                 <$(0x15)>
    div     .16bit      <$(80)>
    ; EXR0 quotient(row), EXR1 reminder(col)
    cmp     .16bit      <%exr0>,                                    <$(24)>
    jbe                 <%cb>,                                      <.scroll>

    xor     .16bit      <%exr1>,                                    <%exr1>
    inc     .16bit      <%exr0>
    mul     .16bit      <$(80)>
    SETCUSP
    REFRESH
    jmp                 <%cb>,                                      < .exit>

    .scroll:
        ; move content (scroll up)
        mov .64bit      <%db>,                                      <$(0xB8000)>
        xor .64bit      <%dp>,                                      <%dp>
        mov .64bit      <%eb>,                                      <$(0xB8000 + 80)>
        xor .64bit      <%ep>,                                      <%ep>
        mov .64bit      <%fer3>,                                    <$(2000 - 80)>
        movs

        ; clear last line
        mov .64bit      <%fer3>,                                    <$(80)>
        mov .64bit      <%eb>,                                      <$(0xB8000)>
        mov .64bit      <%ep>,                                      <$(2000 - 80)>
        xor .64bit      <%dp>,                                      <%dp>
        .scroll.loop:
            mov .8bit   <*1&8(%eb, %ep, %dp)>,                      <$(' ')>
            inc .64bit  <%dp>
            loop        <%cb>,                                      <.scroll.loop>

        mov .16bit      <%exr0>,                                    <$(2000 - 80)>
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
        mov .8bit       <%r2>,                                      <*1&8(%db, %dp, $(0))>

        cmp .8bit       <%r2>,                                      <$(0)>
        je              <%cb>,                                      <.exit>

        cmp .8bit       <%r2>,                                      <$(0x0A)>
        jne             <%cb>,                                      <.skip_newline>

        .newline:
        call            <%cb>,                                      <_newline>
        mov .64bit      <%fer3>,                                    <.last_offset>
        mov .16bit      <*1&16($(0), %fer3, $(0))>,                 <%exr0>
        jmp             <%cb>,      <.end>

        .skip_newline:
        xor .8bit       <%r3>,                                      <%r3>
        mov .64bit      <%fer3>,                                    <.last_offset>
        mov .16bit      <%exr0>,                                    <*1&16($(0), %fer3, $(0))>
        call            <%cb>,                                      <_putc>

        inc .16bit      <%exr0>
        cmp .16bit      <%exr0>,                                    <$(2000)>
        je              <%cb>,                                      <.newline>

        mov .16bit      <*1&16($(0), %fer3, $(0))>,                 <%exr0>
        SETCUSP

        .end:
        inc .64bit      <%dp>
        jmp             <%cb>,                                      <.loop>

    .exit:
    popall
    ret

.last_offset:
    .16bit_data < 0 >

; _print_num(%fer0)
_print_num:
    pushall
    ; record occurrences of digits
    xor .64bit          <%fer2>,                                    <%fer2>
    .loop:
        div .64bit      <$(10)>
        ; %fer0 ==> ori
        ; %fer1 ==> reminder
        mov  .64bit     <%fer3>,                                    <%fer1>
        add  .64bit     <%fer3>,                                    <$('0')>
        push .64bit     <%fer3>

        inc .64bit      <%fer2>

        cmp .64bit      <%fer0>,                                    <$(0x00)>
        jne             <%cb>,                                      <.loop>

    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.cache>

    mov .64bit          <%fer3>,                                    <%fer2>
    .loop_pop:
        pop .64bit      <%fer0>
        mov .8bit       <*1&8(%db, %dp, $(0))>,                     <%r0>
        inc .64bit      <%dp>
        loop            <%cb>,                                      <.loop_pop>

    mov .8bit           <*1&8(%db, %dp, $(0))>,                     <$(0)>
    mov .64bit          <%dp>,                                      <.cache>
    call                <%cb>,                                      <_puts>

    popall
    ret

    .cache:
        .resvb < 16 >

_start:
    mov .64bit          <%sp>,                                      <$(0xFFF)>
    mov .64bit          <%sb>,                                      <_stack_frame>
    mov .64bit          <*1&64($(0xA0000), $(16 * 128),  $(8))>,    <_int_rtc>
    mov .64bit          <*1&64($(0xA0000), $(16 * 5), $(8))>,       <_int_kb_abort>
    ; out .64bit          <$(RTC_TIME)>,                              <$(1560600000)>
    out .64bit          <$(RTC_INT)>,                               <$(0x4E2080)>   ; 1s, 0x80

    .inf_loop:
        int             <$(0x14)>
        cmp .8bit       <%r0>,                                      <$('q')>
        jne             <%cb>,                                      <.inf_loop>

    xor .64bit          <%fer0>,                                    <%fer0>
    hlt

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

; check if a year is a leap year
; is_leap_year(year ==> %fer0)->bool
is_leap_year:
    push .64bit         <%fer1>
    push .64bit         <%fer2>

    mov .64bit          <%fer2>,                                    <%fer0>

    ; %fer0 % 400 == 0 ==> 1
    div .64bit          <$(400)>
    cmp .64bit          <%fer1>,                                    <$(0)>
    je                  <%cb>,                                      <.return1>

    ; %fer0 % 100 == 0 ==> 0
    mov .64bit          <%fer0>,                                    <%fer2>
    div .64bit          <$(100)>
    cmp .64bit          <%fer1>,                                    <$(0)>
    je                  <%cb>,                                      <.return0>

    ; %fer0 % 4 == 0 ==> 1
    mov .64bit          <%fer0>,                                    <%fer2>
    div .64bit          <$(4)>
    cmp .64bit          <%fer1>,                                    <$(0)>
    je                  <%cb>,                                      <.return1>

    jmp                 <%cb>,                                      <.return0>

    .return1:
    mov .64bit          <%fer0>,                                    <$(1)>
    jmp                 <%cb>,                                      <.return>

    .return0:
    mov .64bit          <%fer0>,                                    <$(0)>

    .return:
    pop .64bit          <%fer2>
    pop .64bit          <%fer1>
    ret

; Function to get the number of days in a month for a given year
; get_days_in_month(month ==> %fer0, year ==> %fer1)-> days ==> %fer0
get_days_in_month:
    cmp .64bit          <%fer0>,                                    <$(1)>
    je                  <%cb>,                                      <.month1>
    cmp .64bit          <%fer0>,                                    <$(2)>
    je                  <%cb>,                                      <.month2>
    cmp .64bit          <%fer0>,                                    <$(3)>
    je                  <%cb>,                                      <.month3>
    cmp .64bit          <%fer0>,                                    <$(4)>
    je                  <%cb>,                                      <.month4>
    cmp .64bit          <%fer0>,                                    <$(5)>
    je                  <%cb>,                                      <.month5>
    cmp .64bit          <%fer0>,                                    <$(6)>
    je                  <%cb>,                                      <.month6>
    cmp .64bit          <%fer0>,                                    <$(7)>
    je                  <%cb>,                                      <.month7>
    cmp .64bit          <%fer0>,                                    <$(8)>
    je                  <%cb>,                                      <.month8>
    cmp .64bit          <%fer0>,                                    <$(9)>
    je                  <%cb>,                                      <.month9>
    cmp .64bit          <%fer0>,                                    <$(10)>
    je                  <%cb>,                                      <.month10>
    cmp .64bit          <%fer0>,                                    <$(11)>
    je                  <%cb>,                                      <.month11>
    cmp .64bit          <%fer0>,                                    <$(12)>
    je                  <%cb>,                                      <.month12>

    .month1:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month2:
    mov .64bit          <%fer0>,                                    <%fer1>
    call                <%cb>,                                      <is_leap_year>
    cmp .64bit          <%fer0>,                                    <$(1)>
    je                  <%cb>,                                      <.leap>

    .not_leap:
        mov .64bit      <%fer0>,                                    <$(28)>
        jmp             <%cb>,                                      <.end>

    .leap:
        mov .64bit      <%fer0>,                                    <$(29)>
        jmp             <%cb>,                                      <.end>

    .month3:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month4:
    mov .64bit          <%fer0>,                                    <$(30)>
    jmp                 <%cb>,                                      <.end>

    .month5:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month6:
    mov .64bit          <%fer0>,                                    <$(30)>
    jmp                 <%cb>,                                      <.end>

    .month7:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month8:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month9:
    mov .64bit          <%fer0>,                                    <$(30)>
    jmp                 <%cb>,                                      <.end>

    .month10:
    mov .64bit          <%fer0>,                                    <$(31)>
    jmp                 <%cb>,                                      <.end>

    .month11:
    mov .64bit          <%fer0>,                                    <$(30)>
    jmp                 <%cb>,                                      <.end>

    .month12:
    mov .64bit          <%fer0>,                                    <$(31)>

    .end:
    ret

; determine_the_year(total_days => %fer0)->(%fer0 => year, %fer1 => total_days)
determine_the_year:
    push .64bit         <%fer2>
    push .64bit         <%fer3>

    mov .64bit          <%fer1>,                                    <%fer0>
    mov .64bit          <%fer2>,                                    <$(1970)>

    ; %fer1 => total_days, %fer2 => year

    .loop:
        mov .64bit      <%fer0>,                                    <%fer2>
        call            <%cb>,                                      <is_leap_year>
        cmp .64bit      <%fer0>,                                    <$(1)>
        je              <%cb>,                                      <.is_leap>

        .is_not_leap:
        mov .64bit      <%fer3>,                                    <$(365)>
        jmp             <%cb>,                                      <.end_is_leap_cmp>

        .is_leap:
        mov .64bit      <%fer3>,                                    <$(366)>

        .end_is_leap_cmp:
        cmp .64bit      <%fer1>,                                    <%fer3>
        jl              <%cb>,                                      <.break>

        sub .64bit      <%fer1>,                                    <%fer3>
        inc .64bit      <%fer2>
        jmp             <%cb>,                                      <.loop>
    .break:

    mov .64bit          <%fer0>,                                    <%fer2>

    pop .64bit          <%fer3>
    pop .64bit          <%fer2>
    ret

; determine_the_month(total_days => %fer0, year => %fer1)->(%fer0 => month, %fer1 => total_days)
determine_the_month:
    push .64bit         <%fer2>
    push .64bit         <%fer3>

    ; %fer2 => month counter
    mov .64bit          <%fer2>,                                    <$(1)>
    ; total_days => %fer3
    mov .64bit          <%fer3>,                                    <%fer0>

    .loop:
        mov .64bit      <%fer0>,                                    <%fer2>
        call            <%cb>,                                      <get_days_in_month> ; => %fer0
        cmp .64bit      <%fer3>,                                    <%fer0>
        jl              <%cb>,                                      <.break>

        sub .64bit      <%fer3>,                                    <%fer0>
        inc .64bit      <%fer2>

        jmp             <%cb>,                                      <.loop>

    .break:

    mov .64bit          <%fer0>,                                    <%fer2>
    mov .64bit          <%fer1>,                                    <%fer3>

    pop .64bit          <%fer3>
    pop .64bit          <%fer2>
    ret


; _print_time(%fer0)
_print_time:
    pushall

    ; Calculate total days and remaining seconds
    div .64bit          SECONDS_IN_DAY
    ; total_days => %fer15, remaining_seconds => %fer0
    mov .64bit          <%fer15>,                                   <%fer0>
    mov .64bit          <%fer0>,                                    <%fer1>

    ; Calculate current time
    div .64bit          SECONDS_IN_HOUR
    ; hour => %fer14, remaining_seconds => %fer1
    mov .64bit          <%fer14>,                                   <%fer0>
    mov .64bit          <%fer0>,                                    <%fer1>
    div .64bit          SECONDS_IN_MINUTE
    ; minute => %fer13, second => %fer12
    mov .64bit          <%fer13>,                                   <%fer0>
    mov .64bit          <%fer12>,                                   <%fer1>

    ; total_days => %fer15
    ; hour => %fer14
    ; minute => %fer13
    ; second => %fer12

    ; determine_the_year(total_days => %fer0)->(%fer0 => year, %fer1 => total_days)
    ; determine_the_month(total_days => %fer0, year => %fer1)->(%fer0 => month, %fer1 => total_days)

    mov .64bit          <%fer0>,                                    <%fer15>
    call                <%cb>,                                      <determine_the_year>
    mov .64bit          <%fer11>,                                   <%fer0> ; year

    mov .64bit          <%fer0>,                                    <%fer1>
    mov .64bit          <%fer1>,                                    <%fer11>
    call                <%cb>,                                      <determine_the_month>
    mov .64bit          <%fer10>,                                   <%fer0> ; month
    mov .64bit          <%fer9>,                                    <%fer1> ; days
    inc .64bit          <%fer9>     ; days++

    ; year   => %fer11
    ; month  => %fer10
    ; day    => %fer9
    ; hour   => %fer14
    ; minute => %fer13
    ; second => %fer12

    xor .64bit          <%db>,                                      <%db>

    ; print day
    mov .64bit          <%fer0>,                                    <%fer9>
    call                <%cb>,                                      <_print_num>

    mov .64bit          <%dp>,                                      <.space>
    call                <%cb>,                                      <_puts>

    ; print month
    mov .64bit          <%db>,                                      <.months>
    mov .64bit          <%dp>,                                      <%fer10>
    dec .64bit          <%dp>
    lea                 <%dp>,                                      <*4&8(%dp, $(0), $(0))>
    lea                 <%fer0>,                                    <*1&8(%db, %dp, $(0))>
    call                <%cb>,                                      <_puts>

    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.space>
    call                <%cb>,                                      <_puts>

    ; print year
    mov .64bit          <%fer0>,                                    <%fer11>
    call                <%cb>,                                      <_print_num>

    mov .64bit          <%dp>,                                      <.space>
    call                <%cb>,                                      <_puts>

    ; print time
    mov .64bit          <%fer0>,                                    <%fer14>
    call                <%cb>,                                      <_print_num>

    mov .64bit          <%dp>,                                      <.col>
    call                <%cb>,                                      <_puts>

    mov .64bit          <%fer0>,                                    <%fer13>
    call                <%cb>,                                      <_print_num>

    mov .64bit          <%dp>,                                      <.col>
    call                <%cb>,                                      <_puts>

    mov .64bit          <%fer0>,                                    <%fer12>
    call                <%cb>,                                      <_print_num>

    popall
    ret

    .col:
    .string < ":" >
    .8bit_data < 0 >

    .space:
    .string < " " >
    .8bit_data < 0 >

    .months:
    .string < "Jan" >
    .8bit_data < 0 >
    .string < "Feb" >
    .8bit_data < 0 >
    .string < "Mar" >
    .8bit_data < 0 >
    .string < "Apr" >
    .8bit_data < 0 >
    .string < "May" >
    .8bit_data < 0 >
    .string < "Jun" >
    .8bit_data < 0 >
    .string < "Jul" >
    .8bit_data < 0 >
    .string < "Aug" >
    .8bit_data < 0 >
    .string < "Sep" >
    .8bit_data < 0 >
    .string < "Oct" >
    .8bit_data < 0 >
    .string < "Nov" >
    .8bit_data < 0 >
    .string < "Dec" >
    .8bit_data < 0 >

_int_rtc:
    xor .64bit          <%db>,                                      <%db>
    mov .64bit          <%dp>,                                      <.message>
    call                <%cb>,                                      <_puts>
    in .64bit           <$(RTC_TIME)>,                              <%fer0>

    ; output current time
    call                <%cb>,                                      <_print_time>

    mov .64bit          <%dp>,                                      <.message.tail>
    call                <%cb>,                                      <_puts>
    iret
    .message:
    .string < "Current time: " >
    .8bit_data < 0 >
    .message.tail:
    .string < " UTC\n" >
    .8bit_data < 0 >

_stack_frame:
.resvb < 16 - ( (@ - @@) % 16 ) >
