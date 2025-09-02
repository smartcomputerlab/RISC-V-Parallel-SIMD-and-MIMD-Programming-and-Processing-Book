# avg_i32_rvv.s
# int32_t avg_i32_rvv(const int32_t *src, size_t n);
# RV64 + V (RVV 1.0). Returns 0 if n == 0.
    .text
    .globl  avg_i32_rvv
    .type   avg_i32_rvv, @function
avg_i32_rvv:
    beqz    a1, .ret_zero           # if (n==0) return 0

    mv      a2, a1                  # save original n for the final divide
    li      t5, 0                   # 64-bit scalar accumulator sum = 0

1:  beqz    a1, 2f                  # while (remaining > 0)

    # Set VL for remaining elements, SEW=32, LMUL=1
    vsetvli t0, a1, e32, m1, ta, ma     # t0 := vl

    # Load a chunk of int32
    vle32.v v0, (a0)

    # Widen to e64 (LMUL=m2) and reduce-sum to a 64-bit scalar
    vsetvli x0, t0, e64, m2, ta, ma     # same element count in widened type
    vsext.vf2 v2, v0                    # v2 (e64,m2) = sign-extend(v0)
    vmv.v.i  v4, 0                      # zero seed
    vredsum.vs v2, v2, v4               # v2[0] = sum of widened lanes
    vmv.x.s  t2, v2                     # t2 = chunk sum (64-bit)

    add      t5, t5, t2                 # accumulate

    # Advance pointers/counters
    slli     t1, t0, 2                  # bytes = vl * 4
    add      a0, a0, t1                 # src += vl
    sub      a1, a1, t0                 # remaining -= vl
    bnez     a1, 1b

2:  # Average = sum / original_n  (truncates toward zero)
    mv       a0, t5                     # a0 = 64-bit sum
    # a2 holds original n (>0)
    div      a0, a0, a2                 # signed divide, trunc toward zero
    sext.w   a0, a0                     # return as int32_t (RV64 sign-extend)
    ret

.ret_zero:
    li       a0, 0
    ret
    .size avg_i32_rvv, .-avg_i32_rvv

