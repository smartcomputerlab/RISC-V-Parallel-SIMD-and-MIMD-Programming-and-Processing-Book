# rotate90_rvv.s
# a0 = src (u8*)
# a1 = dst (u8*)
# a2 = N   (size_t, width==height)
#
# dst[(c*N + (N-1-r))*3 + k] = src[((r*N + c)*3) + k], k=0..2

    .text
    .globl  rotate90_rvv
    .align  2
rotate90_rvv:
    beqz    a2, .Lret                # if (N==0) return
    li      t5, 3                    # bytes per pixel
    mul     a3, a2, t5               # a3 = row_stride = N*3 (bytes)
    mv      a4, a3                   # a4 = dst column stride = N*3
    li      t0, 0                    # r = 0
.Lrow:
    beq     t0, a2, .Lret            # for (r = 0; r < N; ++r)
    # srow = src + r*row_stride
    mul     t2, t0, a3
    add     t2, a0, t2               # t2 = srow
    # dptr = dst + (N-1-r)*3
    addi    t3, a2, -1               # N-1
    sub     t3, t3, t0               # N-1-r
    mul     t3, t3, t5               # *3
    add     t3, a1, t3               # t3 = dptr (base for c=0)
    mv      t6, a2                   # rem = N columns left
.Lcol:
    beqz    t6, .Lnext_row
    # Set vl for this chunk (SEW=8, LMUL=1)
    vsetvli t1, t6, e8, m1, ta, ma   # t1 = vl
    # Load vl RGB triplets from source row (unit stride)
    # -> v0=R, v1=G, v2=B
    vlseg3e8.v  v0, (t2)
    # Store to rotated column with stride = N*3 bytes
    vssseg3e8.v v0, (t3), a4
    # srow += 3*vl  (t4 = 2*vl + vl)
    slli    t4, t1, 1                # t4 = 2*vl
    add     t4, t4, t1               # t4 = 3*vl
    add     t2, t2, t4
    # dptr += (N*3)*vl
    mul     t4, t1, a4               # t4 = vl * (N*3)
    add     t3, t3, t4
    sub     t6, t6, t1               # rem -= vl
    j       .Lcol

.Lnext_row:
    addi    t0, t0, 1                # r++
    j       .Lrow

.Lret:
    ret

