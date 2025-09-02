    .section .rodata
fmt_row:    .asciz "%4ld %4ld %4ld\n"

A:
    .quad 1, 2, 3
    .quad 4, 5, 6
    .quad 7, 8, 9

B:
    .quad 9, 8, 7
    .quad 6, 5, 4
    .quad 3, 2, 1

    .section .bss
    .align 3                # 8-byte align
C:
    .skip 8*9               # space for 3x3 int64 result

    .section .text
    .globl _start
_start:
    # Prologue: save ra, s0–s4. Keep stack 16-byte aligned (48 bytes total).
    addi sp, sp, -48
    sd   ra, 40(sp)
    sd   s0, 32(sp)
    sd   s1, 24(sp)
    sd   s2, 16(sp)
    sd   s3, 8(sp)
    sd   s4, 0(sp)

    # Base pointers in callee-saved regs
    la   s0, A
    la   s1, B
    la   s2, C
    li   s3, 3               # N = 3 (matrix dimension), callee-saved

    # ---- Compute C = A * B (scalar) ----
    li   t0, 0               # i
outer_i:
    bge  t0, s3, done_compute

    li   t1, 0               # j
outer_j:
    bge  t1, s3, next_i

    li   t2, 0               # sum
    li   t3, 0               # k
inner_k:
    bge  t3, s3, store_C

    # A[i][k]
    mul  t5, t0, s3          # t5 = i*3
    add  t5, t5, t3          # t5 = i*3 + k
    slli t5, t5, 3           # *8 bytes
    add  t6, s0, t5
    ld   t4, 0(t6)           # t4 = A[i][k]

    # B[k][j]
    mul  t5, t3, s3          # t5 = k*3
    add  t5, t5, t1          # t5 = k*3 + j
    slli t5, t5, 3
    add  t6, s1, t5
    ld   t5, 0(t6)           # t5 = B[k][j]

    mul  t4, t4, t5          # t4 = A[i][k]*B[k][j]
    add  t2, t2, t4          # sum += t4

    addi t3, t3, 1
    j    inner_k

store_C:
    # C[i][j] = sum
    mul  t5, t0, s3
    add  t5, t5, t1
    slli t5, t5, 3
    add  t6, s2, t5
    sd   t2, 0(t6)

    addi t1, t1, 1
    j    outer_j

next_i:
    addi t0, t0, 1
    j    outer_i

done_compute:
    # ---- Print C row by row ----
    li   s4, 0               # row counter in callee-saved reg
print_rows:
    bge  s4, s3, finish

    mul  t5, s4, s3
    slli t5, t5, 3           # byte offset = (row*3)*8
    add  t6, s2, t5

    ld   a1, 0(t6)
    ld   a2, 8(t6)
    ld   a3, 16(t6)

    la   a0, fmt_row
    call printf              # uses caller-saved regs only; s3/s4 preserved

    addi s4, s4, 1
    j    print_rows

finish:
    li   a0, 0
    call exit                # exit(0) — end cleanly regardless of ra

    # (No fallthrough; exit() does not return)

    # Epilogue, in case exit() were replaced with 'ret'
    ld   ra, 40(sp)
    ld   s0, 32(sp)
    ld   s1, 24(sp)
    ld   s2, 16(sp)
    ld   s3, 8(sp)
    ld   s4, 0(sp)
    addi sp, sp, 48
    ret

