# integrate_quarter_circle_rvv.S (FIXED)
# double integrate_quarter_circle_rvv_u64(size_t i0, size_t count, size_t steps);
# S = sum_{i=i0}^{i0+count-1} sqrt(1 - x^2), x = (i + 0.5)/steps
# Vector math: FP32; Accumulation: FP64 (double)
# Robust reduction: store vy to stack buffer and sum with scalar FP adds.

    .text
    .align  2
    .globl  integrate_quarter_circle_rvv_u64
    .type   integrate_quarter_circle_rvv_u64, @function

integrate_quarter_circle_rvv_u64:
    # Prologue
    addi    sp, sp, -16
    sd      ra, 0(sp)
    # a0=i0, a1=count, a2=steps
    # FP consts: 1.0f, 0.5f, 0.0f
    li      t0, 0x3f800000         # 1.0f
    fmv.w.x ft1, t0
    li      t0, 0x3f000000         # 0.5f
    fmv.w.x ft2, t0
    fmv.w.x ft4, zero              # 0.0f
    # invN = 1.0f / (float)steps  -> ft0
    fcvt.s.wu ft0, a2
    fdiv.s    ft0, ft1, ft0
    # acc (double) in fa0 = 0.0
    fmv.d.x   fa0, zero
1:  beqz    a1, 2f                 # while (count)
    # Set VL for remaining elements: e32, m1  (float lanes)
    vsetvli x0, a1, e32, m1, ta, ma
    csrr    t1, vl                  # t1 = VL
    # x0f = ((float)i0 + 0.5f) * invN
    fcvt.s.wu ft3, a0
    fadd.s    ft3, ft3, ft2
    fmul.s    ft3, ft3, ft0
    # lane ids 0..VL-1  -> float
    vid.v        v0
    vfcvt.f.xu.v v1, v0             # v1 = float(lane)
    # x = lane*invN + x0f
    vfmul.vf  v1, v1, ft0
    vfadd.vf  v1, v1, ft3
    # t = 1.0f - x*x
    vfmul.vv  v2, v1, v1            # x*x
    vfrsub.vf v2, v2, ft1           # 1.0f - (x*x)
    # clamp negatives: if (t < 0) t = 0 else keep t
    vfmv.v.f  v3, ft4               # v3 = 0.0f vector
    vmflt.vf  v0, v2, ft4           # mask where t<0
    vmerge.vvm v2, v2, v3, v0       # <-- FIXED ORDER: vd = mask ? 0 : t
    # y = sqrt(t)
    vfsqrt.v  v4, v2
    # robust reduction: store vy to stack, sum with scalar loads
    slli     t2, t1, 2              # bytes = VL * sizeof(float)
    sub      sp, sp, t2             # allocate temp buffer
    mv       t3, sp                 # t3 = buf*
    vse32.v  v4, (t3)
    mv       t4, t1                 # t4 = remaining
3:  beqz     t4, 4f
    flw      ft6, 0(t3)             # load one float
    fcvt.d.s ft7, ft6               # widen to double
    fadd.d   fa0, fa0, ft7          # acc +=
    addi     t3, t3, 4
    addi     t4, t4, -1
    j        3b
4:
    add      sp, sp, t2             # free temp buffer
    # advance i0 += VL; count -= VL
    add      a0, a0, t1
    sub      a1, a1, t1
    bnez     a1, 1b
2:  ld      ra, 0(sp)
    addi     sp, sp, 16
    ret

    .size integrate_quarter_circle_rvv_u64, .-integrate_quarter_circle_rvv_u64

