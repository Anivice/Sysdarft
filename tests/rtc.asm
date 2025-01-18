.equ 'RTC_TIME', '0x70'
.equ 'RTC_INT', '0x71'
.org 0xC1800

_start:
    mov .64bit <%sp>, <$(0xFFF)>                            ; setup stack frame size
    mov .64bit <%sb>, <_stack_frame>                        ; setup stack frame location
    mov .64bit <*1&64($(0xA0000), $(16 * 128),  $(8))>,  <_int_rtc> ; set 128 as RTC interrupt
    mov .64bit <*1&64($(0xA0000), $(16 * 5), $(8))>, <_int_kb_abort>

    out .64bit <$(RTC_INT)>, <$(0x9C4080)> ; 1s, 0x80

    _inf_loop:
        int <$(0x14)>
        cmp .8bit <%r0>, <$('q')>
        jne <%cb>, <_inf_loop>

    hlt

_int_kb_abort:
    xor .64bit <%fer2>, <%fer2>
    xor .64bit <%fer0>, <%fer0>
    mov .64bit <%fer1>, < _message >

    .loop_message:
        mov .8bit <%r0>, <*1&8(%fer1, %fer2, $(0))>
        inc .64bit <%fer2>,
        cmp .8bit <%r0>, <$(0)>
        je <%cb>,  < .loop_msg_end >
        int <$(0x10)>
        jmp <%cb>, < .loop_message >
    .loop_msg_end:
        int <$(0x13)>
        int <$(0x17)>
    iret

_int_rtc:
    in .64bit <$(RTC_TIME)>, <%fer0>
    mov .64bit <%fer2>, <$(0x00)>

    .loop_rtc_start:
        mov .64bit <%fer1>, <$(10)>
        div .64bit <%fer1>
        ; %fer0 ==> ori
        ; %fer1 ==> reminder
        mov .64bit <%fer3>, <%fer0>
        mov .64bit <%fer4>, <%fer1>

        mov .64bit <%fer0>, <%fer1>
        add .64bit <%fer0>, <$('0')>
        push .64bit <%fer0>
        inc .64bit <%fer2>

        mov .64bit <%fer1>, <%fer4>
        mov .64bit <%fer0>, <%fer3>

        cmp .64bit <%fer0>, <$(0x00)>
        jne <%cb>, <.loop_rtc_start>

    mov .64bit <%fer3>, <%fer2>
    .loop_print:
        pop .64bit <%fer0>
        int <$(0x10)>
        loop <%cb>, <.loop_print>
    int <$(0x13)>
    iret

_message:
    .string < "Keyboard Interrupt called!" >
    .8bit_data < 0 >

_stack_frame:
.resvb < 16 - ( (@ - @@) % 16 ) >
