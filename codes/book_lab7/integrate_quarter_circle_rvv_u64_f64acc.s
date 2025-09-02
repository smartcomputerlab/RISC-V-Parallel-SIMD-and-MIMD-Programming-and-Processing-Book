# integrate_quarter_circle_rvv_u64_f64acc(i0, count, steps)
#   a0 = i0     (u64, start index)
#   a1 = count  (u64, number of rectangles)
#   a2 = steps  (u64, total steps)
# Returns fa0 = double(sum_y), where x = (i + 0.5f) / steps and
#               y = sqrtf(max(0, 1 - x*x)); sum_y = Σ y
# Vector math in float (e32), accumulation in double to avoid float stagnation.
# Build: -march=rv64gcv -mabi=lp64d

    .text
    .align  2
    .globl  integrate_quarter_circle_rvv_u64_f64acc
    .type   integrate_quarter_circle_rvv_u64_f64acc, @function

integrate_quarter_circle_rvv_u64_f64acc:
    # acc (double) in fa1 = 0.0
    fmv.d.x   fa1, x0
    # FP32 consts: ft1=1.0f, ft2=0.5f, ft0=0.0f
    la        t0, .LC_ONE_F
    flw       ft1, 0(t0)
    la        t1, .LC_HALF_F
    flw       ft2, 0(t1)
    fmv.w.x   ft0, x0
    # dx = 1.0f / (float)steps -> ft3
    fcvt.s.lu ft4, a2          # (float)steps
    fdiv.s    ft3, ft1, ft4    # dx

.Lloop:
    beqz      a1, .Ldone
    # Set VL for this chunk (e32 float lanes)
    vsetvli   t2, a1, e32, m1, ta, ma
    csrr      t3, vl           # VL
    # x0_f = ((float)a0 + 0.5f) * dx
    fcvt.s.lu ft5, a0
    fadd.s    ft5, ft5, ft2
    fmul.s    ft5, ft5, ft3
    # x = x0_f + (lane_id * dx)
    vid.v     v0                       # lane ids (u32)
    vfcvt.f.xu.v v8, v0                # float
    vfmul.vf  v8, v8, ft3              # * dx
    vfadd.vf  v8, v8, ft5              # + x0_f
    # t = max(0, 1 - x*x)
    vfmul.vv  v9,  v8, v8              # x^2
    vfrsub.vf v10, v9, ft1             # 1 - x^2
    vfmax.vf  v10, v10, ft0            # clamp >= 0
    # y = sqrt(t)
    vfsqrt.v  v11, v10
    # partial = sum(y) in float, then widen to double and accumulate
    vfmv.v.f  v12, ft0                 # 0.0f
    vfredsum.vs v12, v11, v12          # v12[0] = sum(y)
    vfmv.f.s  ft6, v12                 # ft6 = float partial
    fcvt.d.s  ft7, ft6                 # widen to double
    fadd.d    fa1, fa1, ft7            # acc += (double)partial
    # advance
    add       a0, a0, t3
    sub       a1, a1, t3
    bnez      a1, .Lloop

.Ldone:
    fmv.d     fa0, fa1
    ret
    .size integrate_quarter_circle_rvv_u64_f64acc, .-integrate_quarter_circle_rvv_u64_f64acc
    .section .rodata, "a", @progbits
    .align 2
.LC_ONE_F:   .float 1.0
.LC_HALF_F:  .float 0.5

