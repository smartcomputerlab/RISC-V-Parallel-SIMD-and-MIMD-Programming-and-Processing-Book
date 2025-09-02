    .text
    .globl vector_int_add_rvv
vector_int_add_rvv:
    beqz    a3, .Ldone            # if n == 0, return

.Lloop:
    # Choose VL for remaining elements, e32 lanes, LMUL=1.
    # TA/MA: tail-agnostic, mask-agnostic (simple and fast).
    vsetvli t0, a3, e32, m1, ta, ma   # t0 := VL (number of 32-bit lanes we'll process)
    # Load a chunk from a and b
    vle32.v v0, (a0)               # v0 <- a[i .. i+VL-1] (32-bit integers)
    vle32.v v1, (a1)               # v1 <- b[i .. i+VL-1] (32-bit integers)
    # Vector add
    vadd.vv v2, v0, v1             # v2 <- v0 + v1 (32-bit addition)
    # Store result to c
    vse32.v v2, (a2)
    # Advance pointers by VL * sizeof(int32_t) = VL * 4 (bytes)
    slli    t1, t0, 2              # t1 = VL * 4 (bytes) - CORRECTED
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    # Decrease remaining element count by VL and loop
    sub     a3, a3, t0
    bnez    a3, .Lloop

.Ldone:
    ret

