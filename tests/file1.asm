.org 0xC1800
.lab inf

_start:
    mov .64bit <%sp>, <$(0xFFF)>                            ; setup stack frame size
    mov .64bit <%sb>, <_stack_frame>                        ; setup stack frame location
    call <%cb>, <inf>

_stack_frame:
.resvb < 16 - ( (@ - @@) % 16 ) >
