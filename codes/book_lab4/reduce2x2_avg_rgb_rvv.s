    .text
    .globl  reduce2x2_avg_rgb_rvv
    .align  2

# Function: reduce2x2_avg_rgb
# Arguments:
#   a0 = src pointer
#   a1 = srcStep
#   a2 = dst pointer
#   a3 = dstStep
#   a4 = srcW
#   a5 = srcH

reduce2x2_avg_rgb_rvv:
    # Prologue
    addi sp, sp, -56
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    # Save arguments
    mv s0, a0               # s0 = src
    mv s1, a1               # s1 = srcStep
    mv s2, a2               # s2 = dst
    mv s3, a3               # s3 = dstStep
    # Calculate destination dimensions
    srli s4, a4, 1          # s4 = dstW = srcW >> 1
    srli s5, a5, 1          # s5 = dstH = srcH >> 1
    # Initialize vector configuration for 3 channels
    li t0, 3
    vsetvli zero, t0, e8, m1, ta, ma
    # Outer loop: y from 0 to dstH-1
    li t1, 0                # y = 0
y_loop:
    bge t1, s5, y_done
    # Calculate source row pointers
    slli t2, t1, 1          # t2 = 2*y
    mul t3, t2, s1          # t3 = (2*y) * srcStep
    add t4, s0, t3          # t4 = row0 = src + (2*y)*srcStep
    add t5, t4, s1          # t5 = row1 = row0 + srcStep
    # Calculate output row pointer
    mul t6, t1, s3          # t6 = y * dstStep
    add a6, s2, t6          # a6 = out = dst + y*dstStep
    # Inner loop: x from 0 to dstW-1
    li t2, 0                # x = 0

x_loop:
    bge t2, s4, x_done
    # Calculate source pixel offset (2*x * 3)
    slli t3, t2, 1          # t3 = 2*x
    li t6, 3
    mul t3, t3, t6          # t3 = (2*x) * 3 (byte offset)
    # Calculate pixel pointers
    add a7, t4, t3          # p00 = row0 + (2*x)*3
    addi t0, a7, 3          # p01 = p00 + 3
    add t6, t5, t3          # p10 = row1 + (2*x)*3
    addi ra, t6, 3          # p11 = p10 + 3
    # Load pixels as 8-bit unsigned
    vle8.v v0, (a7)         # Load p00 RGB
    vle8.v v1, (t0)         # Load p01 RGB
    vle8.v v2, (t6)         # Load p10 RGB
    vle8.v v3, (ra)         # Load p11 RGB
    # Convert to 16-bit to avoid overflow during summation
    vsetvli zero, t0, e16, m2, ta, ma  # Switch to 16-bit elements
    vzext.vf2 v4, v0        # Zero-extend 8-bit to 16-bit
    vzext.vf2 v6, v1
    vzext.vf2 v8, v2
    vzext.vf2 v10, v3
    # Sum all 4 pixels (16-bit arithmetic)
    vadd.vv v12, v4, v6     # v12 = p00 + p01
    vadd.vv v14, v8, v10    # v14 = p10 + p11
    vadd.vv v16, v12, v14   # v16 = total sum
    # Add rounding and divide by 4: (sum + 2) >> 2
    li t3, 2
    vadd.vx v18, v16, t3    # sum + 2
    vsrl.vi v18, v18, 2     # >> 2 (divide by 4)
    # Convert back to 8-bit
    vsetvli zero, t0, e8, m1, ta, ma   # Switch back to 8-bit
    vncvt.x.x.w v20, v18    # Narrow 16-bit to 8-bit
    # Calculate output position (x * 3)
    li t3, 3
    mul t6, t2, t3          # t6 = x * 3
    add t0, a6, t6          # t0 = out + x*3
    # Store the averaged pixel
    vse8.v v20, (t0)
    # Next x
    addi t2, t2, 1
    j x_loop

x_done:
    # Next y
    addi t1, t1, 1
    j y_loop

y_done:
    # Epilogue
    ld ra, 0(sp)
    ld s0, 8(sp)
    ld s1, 16(sp)
    ld s2, 24(sp)
    ld s3, 32(sp)
    ld s4, 40(sp)
    ld s5, 48(sp)
    addi sp, sp, 56
    ret

