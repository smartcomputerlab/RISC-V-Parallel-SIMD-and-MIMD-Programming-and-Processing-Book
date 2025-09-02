# int32_t dot_i32_rvv(const int32_t *a, const int32_t *b, size_t n);
    .text
    .globl  dot_i32_rvv
    .type   dot_i32_rvv, @function
dot_i32_rvv:
    beqz    a2, .ret0
    li      t5, 0
1:  beqz    a2, 2f
    vsetvli t0, a2, e32, m1, ta, ma
    vle32.v v0, (a0)
    vle32.v v1, (a1)
    vmul.vv v2, v0, v1
    vmv.v.i v3, 0
    vredsum.vs v3, v2, v3        # essential instruction - reduction
    vmv.x.s t1, v3
    add     t5, t5, t1
    slli    t1, t0, 2
    add     a0, a0, t1
    add     a1, a1, t1
    sub     a2, a2, t0
    bnez    a2, 1b
2:  mv      a0, t5
    sext.w  a0, a0          # narrow to int32_t (sign-extend on RV64)
    ret
.ret0:
    li      a0, 0
    ret
    .size dot_i32_rvv, .-dot_i32_rvv

