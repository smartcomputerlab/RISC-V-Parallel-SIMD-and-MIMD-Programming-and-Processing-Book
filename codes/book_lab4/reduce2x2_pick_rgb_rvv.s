    .text
    .globl  reduce2x2_pick_rgb_rvv
    .align  2
reduce2x2_pick_rgb_rvv:
    srli    a4, a4, 1              # a4 = dstW
    srli    a5, a5, 1              # a5 = dstH
    beqz    a4, .done
    beqz    a5, .done
    li      t4, 6                  # src pixel-to-pixel stride when skipping one (2*3)
    li      t6, 3                  # bytes per pixel
    li      t0, 0                  # y = 0
.row:
    beq     t0, a5, .done
    # srow = src + (2*y)*srcStep
    slli    t1, t0, 1              # t1 = 2*y
    mul     t2, t1, a1
    add     t2, a0, t2             # t2 = srow
    # drow = dst + y*dstStep
    mul     t3, t0, a3
    add     t3, a2, t3             # t3 = drow
    mv      t5, a4                 # rem = dstW
.col:
    beqz    t5, .next_row
    vsetvli t1, t5, e8, m1, ta, ma # t1 = vl
    vlsseg3e8.v  v0, (t2), t4      # load selected RGB pixels (stride=6)
    vsseg3e8.v   v0, (t3)          # store contiguous RGB pixels
    # inc_dst = 3*vl  (use t6=3)
    mul     t1, t1, t6             # t1 = 3*vl
    add     t3, t3, t1             # drow += 3*vl
    # inc_src = 6*vl  (use t4=6)
    vsetvli t1, t5, e8, m1, ta, ma # recompute vl (same value) into t1
    mul     t1, t1, t4             # t1 = 6*vl
    add     t2, t2, t1             # srow += 6*vl
    # rem -= vl  (vl is still the same; recompute once more)
    vsetvli t1, t5, e8, m1, ta, ma
    sub     t5, t5, t1
    j       .col

.next_row:
    addi    t0, t0, 1              # y++
    j       .row

.done:
    
