# file: add_printf.S
    .section .rodata
fmt: .asciz "a + b = %ld\n"

    .section .text
    .globl _start
_start:
    li   a0, 7          # a = 7
    li   a1, 5          # b = 5
    add  a2, a0, a1     # a2 = a + b

    # Set up printf(fmt, a2)
    la   a0, fmt        # a0 = &fmt (1st arg)
    mv   a1, a2         # a1 = sum  (2nd arg)
    call printf         # jal ra, printf

    li   a0, 0          # return 0 from main
    ret

