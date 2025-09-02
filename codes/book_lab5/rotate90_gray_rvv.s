    .text
    .globl  rotate90_gray_rvv
    .align  2

rotate90_gray_rvv:
    li      t6, 512               # N
    li      t0, 0                 # r = 0
    mv      t5, t6                # stride bytes for vsse8.v: N
1:  beq     t0, t6, 9f            # if (r == N) return
    # srow = src + r*N
    mul     t1, t0, t6
    add     t2, a0, t1            # t2 = srow
    # dst_ptr = dst + (N-1 - r)   (starting at column 0)
    addi    t3, t6, -1            # N-1
    sub     t3, t3, t0            # N-1-r
    add     t3, a1, t3            # t3 = &dst[0*N + (N-1-r)]
    mv      t4, t6                # remaining columns in this row: rem = N

2:  beqz    t4, 8f                # done this row?

    # Set vl (elements) for this chunk (SEW=8)
    vsetvli t1, t4, e8, m1, ta, ma   # t1 = vl
    mv      a2, t1                   # save vl
    # Load 'vl' bytes from source row
    vle8.v  v0, (t2)
    # Strided store into destination column: stride == N bytes
    vsse8.v v0, (t3), t5
    # Advance pointers: srow += vl; dst_ptr += vl*N; rem -= vl
    add     t2, t2, a2               # srow += vl
    mul     t1, a2, t6               # t1 = vl*N
    add     t3, t3, t1               # dst_ptr += vl*N
    sub     t4, t4, a2               # rem -= vl
    j       2b

8:  addi    t0, t0, 1                # r++
    j       1b

9:  ret

