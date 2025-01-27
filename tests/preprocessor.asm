.org 0xC1800
.equ 'REFRESH', 'int < $(0x18) >'
.equ 'SETCUSP', 'int < $(0x11) >'
.equ 'INTGETC', 'int < $(0x14) >'
.equ 'DISK_SIZE',           '< $(0x136) >'
.equ 'DISK_START_SEC',      '< $(0x137) >'
.equ 'DISK_OPS_SEC_CNT',    '< $(0x138) >'
.equ 'DISK_INPUT',          '< $(0x13A) >'
.equ 'FDA_SIZE',            '< $(0x116) >'
.equ 'FDA_START_SEC',       '< $(0x117) >'
.equ 'FDA_OPS_SEC_CNT',     '< $(0x118) >'
.equ 'FDA_OUTPUT',          '< $(0x119) >'

jmp <%cb>, <_start>

; _putc(%EXR0, linear position, %EXR1, ASCII Code)
_putc:
    pushall
    mov     .64bit <%db>,       <$(0xB8000)>    ; set data base to base of video memory
    push    .16bit <%exr1>                      ; preserve %exr1
    xor     .16bit <%exr1>,     <%exr1>         ; clear %exr1
    xor     .32bit <%her1>,     <%her1>         ; clear %her1
    mov     .64bit <%dp>,       <%fer0>         ; by clearing the rest of the registers
                                                ; %fer0 is the linear address of the display location
    pop     .16bit <%exr0>
    mov     .8bit  <*1&8(%db, %dp, $(0))>, <%r0>

    REFRESH

    popall
    ret

; _newline(%EXR0, linear address)
_newline:
    push .16bit <%exr1>
    push .64bit <%dp>
    push .64bit <%db>
    push .64bit <%ep>
    push .64bit <%eb>
    push .64bit <%fer3>

    div .16bit  <$(80)>
    ; EXR0 quotient, EXR1 reminder
    cmp .16bit  <%exr0>,    <$(24)>
    je          <%cb>,      <.scroll>

    xor .16bit  <%exr1>,    <%exr1>
    inc .16bit  <%exr0>
    mul .16bit  <$(80)>
    SETCUSP
    REFRESH
    jmp         <%cb>,      <.exit>

    .scroll:
        mov .64bit <%db>,   <$(0xB8000)>
        xor .64bit <%dp>,   <%dp>
        mov .64bit <%eb>,   <$(0xB8000 + 25)>
        xor .64bit <%ep>,   <%ep>
        mov .64bit <%fer3>, <$(2000 - 80)>
        movs
        mov .16bit <%exr0>, <$(2000 - 80)>
        SETCUSP
        REFRESH
    .exit:

    pop .64bit <%fer3>
    pop .64bit <%eb>
    pop .64bit <%ep>
    pop .64bit <%db>
    pop .64bit <%dp>
    pop .16bit <%exr1>
    ret

; _puts(%DB:%DP), null terminated string
_puts:
    pushall
    .loop:
        mov .8bit   <%r2>,      <*1&8(%db, %dp, $(0))>      ; R2 is in EXR1

        cmp .8bit   <%r2>,      <$(0)>
        je          <%cb>,      <.exit>

        cmp .8bit   <%r2>,      <$(0x0A)>
        jne         <%cb>,      <.skip_newline>

        call        <%cb>,      <_newline>
        mov .64bit  <%fer3>,    <.last_offset>
        mov .16bit  <*1&16($(0), %fer3, $(0))>, <%exr0>
        jmp         <%cb>,      <.end>

        .skip_newline:
        xor .8bit   <%r3>,      <%r3>
        mov .64bit  <%fer3>,    <.last_offset>
        mov .16bit  <%exr0>,    <*1&16($(0), %fer3, $(0))>
        call        <%cb>,      <_putc>
        inc .16bit  <%exr0>
        mov .16bit  <*1&16($(0), %fer3, $(0))>, <%exr0>
        SETCUSP

        .end:
        inc .64bit  <%dp>
        jmp         <%cb>,      <.loop>

    .exit:
    popall
    ret

.last_offset:
    .16bit_data < 0 >

; _print_num(%fer0)
_print_num:
    pushall

    xor .64bit <%fer2>, <%fer2>       ; record how many digits
    .loop:
        div .64bit <$(10)>
        ; %fer0 ==> ori
        ; %fer1 ==> reminder
        mov .64bit <%fer3>, <%fer1>
        add .64bit <%fer3>, <$('0')>
        push .64bit <%fer3>

        inc .64bit <%fer2>

        cmp .64bit <%fer0>, <$(0x00)>
        jne <%cb>, <.loop>

    xor .64bit <%db> ,<%db>
    mov .64bit <%dp>, <.cache>

    mov .64bit <%fer3>, <%fer2>
    .loop_pop:
        pop .64bit <%fer0>
        mov .8bit <*1&8(%db, %dp, $(0))>, <%r0>
        inc .64bit <%dp>
        loop <%cb>, <.loop_pop>

    mov .8bit <*1&8(%db, %dp, $(0))>, <$(0)>
    mov .64bit <%dp>, <.cache>
    call <%cb>, <_puts>

    popall
    ret

    .cache:
        .resvb < 256 >

_reads:
    pushall
    in .64bit DISK_SIZE, <%fer3>
    popall
    ret

_int_0x02_io_error:
    mov .64bit  <%dp>,  <.message>
    xor .64bit  <%db>,  <%db>
    call        <%cb>,  <_puts>
    INTGETC
    hlt
    .message:
    .string < "IO ERROR!\nPress any key to shutdown..." >
    .8bit_data < 0 >

_start:
    mov .64bit  <%sb>,  <_stack_frame>
    mov .64bit  <%sp>,  <$(0xFFF)>

    ; install error handler
    mov .64bit <*1&64($(0xA0000), $(0x02 * 16), $(8))>, <_int_0x02_io_error>

    ; show welcome message
    mov .64bit  <%dp>,  <.welcome>
    xor .64bit  <%db>,  <%db>
    call        <%cb>,  <_puts>

    INTGETC

    ; read from disk
    mov .64bit  <%dp>,  <.reading_from_disk>
    call        <%cb>,  <_puts>
    call        <%cb>,  <_reads>
    jmp         <%cb>,  <$(@)>

.welcome:
    .string < "Hello!\nThis is Sysdarft!\nPress any key to read from disk\n" >
    .8bit_data < 0 >

.reading_from_disk:
    .string < "Reading from disk...\n" >
    .8bit_data < 0 >

_stack_frame:
    .resvb < 0xFFF >
