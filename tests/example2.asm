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
    xor .64bit <%DP>, <%DP>                         ; set dp = 0
    xor .64bit <%FER1>, <%FER1>                     ; clear counter
    _loop:
        xor .8bit <%R1>, <%R1>
        mov .8bit <%R0>, <*1&8(%DP, $(0), $(0))>    ; move db:dp to R0
        int <$(0x10)>                               ; print R2
        add .64bit <%DP>, <$(1)>                    ; inc dp
        add .64bit <%FER1>, <$(1)>                  ; inc FER1
        cmp .64bit <%FER1>, <$(2048)>               ; see if the data at it end
        jne <%CB>, <_loop>                          ; if not, continue
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
        int <$(0x10)>
        jmp <%cb>, < _loop_message >
    _loop_msg_end:
        int <$(0x13)>
        int <$(0x17)>                                           ; ring the bell
    iret

_message:
    .string < "Keyboard Interrupt called!" >
    .8bit_data < 0 >

_stack_frame:
.resvb < 16 - ( (@ - @@) % 16 ) >
