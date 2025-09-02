
    .text
    .align  2
    .option push
    .option arch, +v
    .globl  rotate_rgb_90ccw_rvv
# a0=src, a1=srcW, a2=srcH, a3=srcStep, a4=dst, a5=dstW, a6=dstH, a7=dstStep
rotate_rgb_90ccw_rvv:
    # Basic sanity: sizes must be positive and match rotation (dstW=srcH, dstH=srcW)
    bge     x0, a1, .Lret              # srcW <= 0
    bge     x0, a2, .Lret              # srcH <= 0
    bne     a5, a2, .Lret              # dstW != srcH  -> bail (optional)
    bne     a6, a1, .Lret              # dstH != srcW  -> bail (optional)
    beqz    a0, .Lret
    beqz    a4, .Lret

    li      a6, 3                      # a6 = 3 (byte stride between channels for vlse/vsse)
    li      t0, 0                      # y = 0

# ================== Row loop over source rows (y = 0..srcH-1) ==================
.Lrow:
    bge     t0, a2, .Lret
    # row0 = src + y*srcStep
    mul     t1, t0, a3
    add     t2, a0, t1                  # t2 = row0 base
    # Destination column index for CCW: dx = dstW - 1 - y
    addi    t3, a5, -1                  # t3 = dstW - 1
    sub     t3, t3, t0                  # t3 = dx
    # Column byte offset = 3*dx
    slli    t4, t3, 1                   # 2*dx
    add     t4, t4, t3                  # t4 = 3*dx
    # processed x (source columns)
    li      t5, 0
# ------------------ Vector loop across the source row (x = 0..srcW-1) ------------------
.Lcol:
    bge     t5, a1, .Lnext_row
    # VL = min(remaining, VLMAX); we just set by remaining
    sub     t1, a1, t5                  # remaining = srcW - processed
    vsetvli t1, t1, e8, m1              # elements = remaining bytes (per channel)
    csrr    t6, vl                      # t6 = VL (number of pixels in this chunk)
    # byte offsets:
    #   src_chunk_base = row0 + 3*processed
    slli    t1, t5, 1                   # 2*processed
    add     t1, t1, t5                  # t1 = 3*processed
    add     t1, t2, t1                  # t1 = src_chunk_base
    #   dst_row_start = dst + (processed)*dstStep
    mul     t3, t5, a7
    add     t3, a4, t3                  # t3 = dst + processed*dstStep
    #   dst_chunk_base for this column dx: add 3*dx
    add     t3, t3, t4           # t3 = base of column (x-start row), at pixel column dx
    # ---------- LOAD channels from source row (stride = 3 bytes) ----------
    vlse8.v  v0, (t1), a6               # v0 <- R[x .. x+VL-1]
    addi     t1, t1, 1
    vlse8.v  v1, (t1), a6               # v1 <- G[x .. x+VL-1]
    addi     t1, t1, 1
    vlse8.v  v2, (t1), a6               # v2 <- B[x .. x+VL-1]
    # ---------- STORE channels down the destination column (stride = dstStep) ----------
    vsse8.v  v0, (t3), a7               # write R at (dy .. dy+VL-1, dx)
    addi     t1, t3, 1
    vsse8.v  v1, (t1), a7               # write G
    addi     t1, t3, 2
    vsse8.v  v2, (t1), a7               # write B
    # Advance processed x by VL
    add     t5, t5, t6
    j       .Lcol

# ------------------ Next source row ------------------
.Lnext_row:
    addi    t0, t0, 1
    j       .Lrow

.Lret:
    ret
    .option pop

