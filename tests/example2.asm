; example.asm
; this code will attempt to read 4 sectors from the disk
; then print it out
; it also handles keyboard interruption
; it will print a notification and ring the bell when triggered
.equ 'HDD_SEC_START', '0x137'
.equ 'HDD_SEC_COUNT', '0x138'
.equ 'HDD_IO', '0x139'
.lab _start, reads, puts, _stack_frame, _int3, _skip_newl, gets
.lab _loop2, _end, _int_kb_abort, _timeout, _message, _loop_message, _loop_msg_end
.org 0xC1800

jmp <%CB>, < _start >

reads:
    pushall
    out .64bit <$(HDD_SEC_START)>, <$(0)>           ; sector read starts from 0
    out .64bit <$(HDD_SEC_COUNT)>, <$(4)>           ; read 4 sector
    mov .64bit <%FER0>, <$(2048)>                   ; ins operation input 2048 bytes of data
    ins .64bit <$(HDD_IO)>                          ; ins from HDD_IO port
    popall
    ret

puts:
    pushall
    mov .64bit <%FER0>, <$(2000)>
    mov .64bit <%DP>, <$(0xB8000)>
    movs

    ; refresh screen
    int <$(0x18)>

    ; move cursor to last position
    mov .16bit <%EXR0>, <$(1999)>
    int <$(0x11)>

    popall
    ret

gets:
    pushall
    _loop2:
       int <$(0x14)>                                ; getc
       cmp .8bit <%R0>, <$('q')>
       je <%cb>, <_end>

       int <$(0x10)>

       jmp <%cb>, <_loop2>
   _end:
   popall
   ret

_start:
    mov .64bit <%sp>, <$(0xFFF)>                            ; setup stack frame size
    mov .64bit <%sb>, <_stack_frame>                        ; setup stack frame location
    mov .64bit <*1&64($(0xA0000), $(16 * 3), $(8))>, <_int3>
    mov .64bit <*1&64($(0xA0000), $(16 * 5), $(8))>, <_int_kb_abort>
    call <%cb>, <reads>                                     ; read disk
    call <%cb>, <puts>                                      ; print disk content
    call <%cb>, <gets>
    hlt

_int3:
    iret

_int_kb_abort:
    xor .64bit <%fer2>, <%fer2>
    xor .64bit <%fer0>, <%fer0>
    mov .64bit <%FER1>, < _message >

    _loop_message:
        mov .8bit <%R0>, <*1&8(%FER1, %FER2, $(0))>
        add .64bit <%FER2>, <$(1)>
        cmp .8bit <%R0>, <$(0)>
        je <%cb>,  < _loop_msg_end >
        int <$(0x10)>       ; putc
        jmp <%cb>, < _loop_message >
    _loop_msg_end:
        int <$(0x13)>       ; newline
        int <$(0x17)>       ; ring the bell
    iret

_message:
    .string < "Keyboard Interrupt called!" >
    .8bit_data < 0 >

_stack_frame:
.resvb < 16 - ( (@ - @@) % 16 ) >
