.org 0xC1800

_start:
    mov .64bit <%FER0>, <$(0x117)>

    out .64bit <%FER0>, <$(0)>
    inc .64bit <%FER0>
    out .64bit <%FER0>, <$(1)>

    mov .64bit <%DP>, <_start>
    mov .64bit <%FER3>, <$(512)>
    outs .64bit <$(0x11A)>
    jmp <%CB>, <$(@)>
