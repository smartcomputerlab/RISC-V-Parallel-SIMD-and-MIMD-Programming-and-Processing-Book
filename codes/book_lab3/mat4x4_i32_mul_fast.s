# mat4x4_i32_mul_fast.S
# void mat4x4_i32_mul_fast(const int32_t *A, const int32_t *B, int32_t *C);
# A, B, C are 4x4 int32, row-major, tight (16 bytes per row).

    .text
    .globl  mat4x4_i32_mul_fast_rvv
    .type   mat4x4_i32_mul_fast_rvv,@function
mat4x4_i32_mul_fast_rvv:
    # a0=A, a1=B, a2=C

    li      t5, 4
    vsetvli t0, t5, e32, m1, ta, ma          # VL=4, SEW=32, LMUL=1

    # ---- Preload B rows (contiguous) into vector regs ----
    vle32.v v4, (a1)                          # v4 = B[0, :]
    addi    t1, a1, 16
    vle32.v v5, (t1)                          # v5 = B[1, :]
    addi    t1, t1, 16
    vle32.v v6, (t1)                          # v6 = B[2, :]
    addi    t1, t1, 16
    vle32.v v7, (t1)                          # v7 = B[3, :]

    li      t6, 0                              # i = 0
1:  bge     t6, t5, 9f                         # for (i=0; i<4; ++i)

    # Load A row i
    slli    t1, t6, 4                          # i*16 bytes
    add     t2, a0, t1
    vle32.v v0, (t2)                            # v0 = [A[i,0..3]]

    # Accumulator v8 = 0
    vmv.v.i v8, 0

    # --- Accumulate A[i,0] * B[0,:] ---
    vrgather.vi v9, v0, 0                       # v9 = broadcast(A[i,0])
    vmul.vv     v10, v4, v9
    vadd.vv     v8,  v8, v10
    # (If supported: vmacc.vv v8, v4, v9)

    # --- Accumulate A[i,1] * B[1,:] ---
    vrgather.vi v9, v0, 1
    vmul.vv     v10, v5, v9
    vadd.vv     v8,  v8, v10
    # vmacc.vv v8, v5, v9

    # --- Accumulate A[i,2] * B[2,:] ---
    vrgather.vi v9, v0, 2
    vmul.vv     v10, v6, v9
    vadd.vv     v8,  v8, v10
    # vmacc.vv v8, v6, v9

    # --- Accumulate A[i,3] * B[3,:] ---
    vrgather.vi v9, v0, 3
    vmul.vv     v10, v7, v9
    vadd.vv     v8,  v8, v10
    # vmacc.vv v8, v7, v9

    # Store C row i (contiguous)
    add     t3, a2, t1                           # &C[i,0]
    vse32.v v8, (t3)

    addi    t6, t6, 1
    j       1b

9:  ret
    .size   mat4x4_i32_mul_fast_rvv, .-mat4x4_i32_mul_fast_rvv

