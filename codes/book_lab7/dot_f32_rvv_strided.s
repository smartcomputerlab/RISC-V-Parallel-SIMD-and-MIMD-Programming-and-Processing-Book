# dot_f32_rvv_strided(const float *a, const float *b, uint64_t n, uint64_t stride_bytes)
# a0=a (unit-stride), a1=b (strided), a2=n, a3=stride_bytes
# return fa0 = sum_{k=0..n-1} a[k] * b[k*stride_bytes/4]
# Build with: -march=rv64gcv -mabi=lp64d

    .text
    .align  2
    .globl  dot_f32_rvv_strided
    .type   dot_f32_rvv_strided, @function

dot_f32_rvv_strided:
    # acc in fa1 = 0.0f
    fmv.w.x   fa1, x0

.Lloop:
    beqz      a2, .Ldone

    # Set VL for remaining n elements, SEW=32 (float)
    vsetvli   t0, a2, e32, m1, ta, ma
    csrr      t1, vl                  # t1 = VL
    # Load contiguous a, strided b
    vle32.v   v0, (a0)                # a chunk
    vlse32.v  v1, (a1), a3            # b chunk with stride (bytes)

    # Multiply and horizontal sum
    vfmul.vv  v2, v0, v1
    # reduce v2 -> scalar partial (float)
    fmv.w.x   ft0, x0                 # 0.0f
    vfmv.v.f  v3, ft0
    vfredsum.vs v3, v2, v3
    vfmv.f.s  ft1, v3                 # partial in ft1
    # acc += partial
    fadd.s    fa1, fa1, ft1
    # advance pointers/counters
    slli      t2, t1, 2               # bytes for a: VL * 4
    add       a0, a0, t2
    mul       t3, t1, a3              # bytes for b: VL * stride
    add       a1, a1, t3
    sub       a2, a2, t1
    bnez      a2, .Lloop

.Ldone:
    fmv.s     fa0, fa1
    ret

    .size dot_f32_rvv_strided, .-dot_f32_rvv_strided

