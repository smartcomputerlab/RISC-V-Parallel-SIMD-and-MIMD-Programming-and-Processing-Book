# file: exit0.S
    .section .text
    .globl _start
_start:
    li   a0, 0        # exit status = 0
    li   a7, 93       # SYS_exit on RISC-V Linux
    ecall             # make the syscall

