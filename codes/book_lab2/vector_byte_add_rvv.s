    .text
    .globl vector_byte_add_rvv
vector_byte_add_rvv:
    beqz    a3, .Ldone            # if n == 0, return

.Lloop:
    # Choose VL for remaining elements, e8 lanes, LMUL=1.
    # TA/MA: tail-agnostic, mask-agnostic (simple and fast).
    vsetvli t0, a3, e8, m1, ta, ma   # t0 := VL (number of lanes we'll process)
    # Load a chunk from a and b
    vle8.v v0, (a0)               # v0 <- a[i .. i+VL-1]
    vle8.v v1, (a1)               # v1 <- b[i .. i+VL-1]
    # Vector add
    vadd.vv v2, v0, v1             # v2 <- v0 + v1
    # Store result to c
    vse8.v v2, (a2)
    # Advance pointers by VL * sizeof(int32_t) = VL * 4
    addi    t1, t0, 1              # t1 = VL * 4 (bytes)
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    # Decrease remaining element count by VL and loop
    sub     a3, a3, t0
    bnez    a3, .Lloop

.Ldone:
    ret

