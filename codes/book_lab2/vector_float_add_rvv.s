
    .text
    .globl vector_float_add_rvv
vector_float_add_rvv:
    beqz    a3, .Ldone

.Lloop:
    # Explicit float configuration (some assemblers support this)
    vsetvli t0, a3, e32, m1, ta, ma   # Process single-precision floats
    vle32.v v0, (a0)
    vle32.v v1, (a1)
    vfadd.vv v2, v0, v1           # Single-precision floating-point add
    vse32.v v2, (a2)
    # Pointer arithmetic (same as integer version)
    slli    t1, t0, 2
    add     a0, a0, t1
    add     a1, a1, t1
    add     a2, a2, t1
    sub     a3, a3, t0
    bnez    a3, .Lloop

.Ldone:
    ret

