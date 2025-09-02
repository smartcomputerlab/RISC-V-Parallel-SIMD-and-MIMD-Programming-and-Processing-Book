   .text
    .align  2
    .option push
    .option arch, +v
    .globl  fft_lowpass_mask_f32_rvv
# void fft_lowpass_mask_f32_rvv(float* X, int N, int k_cut)
# a0 = X (float* interleaved {Re,Im})
# a1 = N (number of complex bins)
# a2 = k_cut (inclusive)

fft_lowpass_mask_f32_rvv:
    # Quick returns
    blez    a1, .Lret             # N <= 0
    bltz    a2, .Lret             # negative cutoff -> keep none (you can change if desired)
    # k = 0; ptr = X
    li      t0, 0                 # t0 = kbase (processed complex elements)
    mv      t2, a0                # t2 = ptr (byte pointer into X)

.Lloop:
    bge     t0, a1, .Lret         # while (kbase < N)
    # Set VL for remaining complex elements
    sub     t1, a1, t0            # remaining = N - kbase
    vsetvli t1, t1, e32, m1       # SEW=32-bit lanes, LMUL=1
    csrr    t4, vl                # t4 = VL (number of complex elements in this chunk)
    # Build bin index vector: idx = kbase + [0..VL-1]
    vid.v   v0                    # v0 = [0,1,2,...] (u32/i32)
    vadd.vx v0, v0, t0            # v0 = idx
    # Compute N - idx
    vrsub.vx v1, v0, a1           # v1 = a1 - v0  (N - idx)
    # min(idx, N-idx)  (unsigned, since indices are non-negative)
    vminu.vv v2, v0, v1           # v2 = min(idx, N-idx)
    # Keep if min(idx, N-idx) <= k_cut
    vmsleu.vx v0, v2, a2          # v0.mask = (v2 <= k_cut) (true where we keep)
    # Load interleaved complex float vectors (unit-stride segmented)
    # v8 = Re[], v9 = Im[]
    vlseg2e32.v v8, (t2)
    # Zero vectors (same type/SEW)
    vxor.vv v10, v10, v10         # v10 = 0.0f bit pattern
    vxor.vv v11, v11, v11         # v11 = 0.0f
    # Select keep-or-zero using the mask v0
    # v8 <- (mask ? v8 : 0), v9 <- (mask ? v9 : 0)
    vmerge.vvm v8, v10, v8, v0
    vmerge.vvm v9, v11, v9, v0
    # Store back interleaved complex
    vsseg2e32.v v8, (t2)
    # Advance kbase and pointer by VL complex elements (each = 2 floats = 8 bytes)
    slli    t5, t4, 3             # t5 = VL * 8 bytes
    add     t2, t2, t5            # ptr += VL * sizeof(complex<float>)
    add     t0, t0, t4            # kbase += VL
    j       .Lloop

.Lret:
    ret
    .option pop

