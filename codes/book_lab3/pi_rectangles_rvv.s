# pi_rectangles_rvv.s
# double pi_rectangles_rvv(size_t N);
# Returns NaN if N == 0.
# Requires: RV64, V (RVV 1.0), and D (float64).

    .text
    .globl  pi_rectangles_rvv
    .type   pi_rectangles_rvv, @function

pi_rectangles_rvv:
    beqz    a0, .ret_nan                # if (N==0) return NaN

    # ---- constants in FP regs ----
    # fa1 = 1.0
    li      t1, 0x3ff0000000000000
    fmv.d.x fa1, t1
    # fa3 = 0.5
    li      t3, 0x3fe0000000000000
    fmv.d.x fa3, t3
    # fa5 = 4.0
    li      t5, 0x4010000000000000
    fmv.d.x fa5, t5
    # fa6 = 0.0 (seed for reductions)
    fmv.d.x fa6, zero

    # dx = 1.0 / (double)N  -> fa2
    fcvt.d.lu fa2, a0                    # fa2 = (double)N
    fdiv.d    fa2, fa1, fa2              # fa2 = 1.0 / N
    # sum = 0.0  (fa0 will hold final sum)
    fmv.d.x   fa0, zero
    # base = 0   (how many i's we've consumed so far)
    li        t2, 0

# while (remaining > 0)
.L_loop:
    beqz    a0, .L_done
    # choose vl for e64 (double lanes)
    vsetvli t0, a0, e64, m1, ta, ma      # t0 = vl
    # v0 <- [0,1,2,...,vl-1]  (indices)
    vid.v   v0
    # v0 <- v0 + base (add scalar base to all indices)
    vadd.vx v0, v0, t2                   # integer add (u64/e64)
    # v1 <- (double) v0
    vfcvt.f.xu.v v1, v0                  # UINT64 -> F64
    # v1 <- (v1 + 0.5) * dx  (midpoint * dx)
    vfadd.vf  v1, v1, fa3                # + 0.5
    vfmul.vf  v1, v1, fa2                # * dx
    # v2 <- x^2
    vfmul.vv  v2, v1, v1
    # v3 <- 1.0 - x^2
    vfrsub.vf v3, v2, fa1                # v3 = fa1 - v2
    # v4 <- sqrt(v3)
    vfsqrt.v  v4, v3                     # essential vector function
    # chunk_sum = vfredsum(v4) with seed 0.0
    vfmv.v.f  v5, fa6                    # seed vector = 0.0
    vfredsum.vs v5, v4, v5               # v5[0] = sum(v4[0..vl-1])
    vfmv.f.s  fa7, v5                    # fa7 = chunk_sum
    # sum += chunk_sum
    fadd.d    fa0, fa0, fa7
    # advance base and remaining
    add       t2, t2, t0                 # base += vl
    sub       a0, a0, t0                 # remaining -= vl
    bnez      a0, .L_loop

# return 4.0 * dx * sum
.L_done:
    fmul.d   fa0, fa0, fa2               # sum * dx
    fmul.d   fa0, fa0, fa5               # * 4.0
    ret

# Return quiet NaN for N==0
.ret_nan:
    li      t0, 0x7ff8000000000000       # quiet NaN bit pattern
    fmv.d.x fa0, t0
    ret
    .size pi_rectangles_rvv, .-pi_rectangles_rvv

