# file: sum_array.S
    .section .rodata
arr:    .quad 10, 20, 30, 40, 50      # 5 elements (64-bit each)
nval:   .quad 5
fmt2:   .asciz "sum = %ld\n"

    .section .text
    .globl _start
_start:
    # a0 = base pointer to arr
    la   a0, arr

    # a1 = number of elements (n)
    la   t0, nval
    ld   a1, 0(t0)

    # a2 = running sum, a3 = loop index i
    li   a2, 0
    li   a3, 0

loop:
    bge  a3, a1, done        # if (i >= n) break
    slli t1, a3, 3           # t1 = i * 8 (bytes per 64-bit)
    add  t2, a0, t1          # t2 = &arr[i]
    ld   t3, 0(t2)           # t3 = arr[i]
    add  a2, a2, t3          # sum += arr[i]
    addi a3, a3, 1           # i++
    j    loop

done:
    # printf("sum = %ld\n", a2)
    la   a0, fmt2
    mv   a1, a2
    call printf

    li   a0, 0
    ret

