# -march=rv32gcv -mabi=ilp32     OR
# -march=rv64gcv -mabi=lp64
# void fill_color_rvv(uint8_t *buffer, uint8_t r, uint8_t g, uint8_t b)

    .text
    .align  2
    .globl  fill_color_rvv
    .type   fill_color_rvv, @function

    .equ    WIDTH,    640
    .equ    HEIGHT,   480
    .equ    ROWBYTES, WIDTH*3        # 1920 bytes per row

fill_color_rvv:
    # a0 = buffer, a1 = r, a2 = g, a3 = b

    mv      t0, a0                   # t0 = base pointer to current row
    li      t5, HEIGHT               # t5 = rows remaining
    beqz    t5, .Ldone

    li      t6, 3                    # t6 = stride (3 bytes between consecutive elements)

.Lrow:
    mv      t2, t0                   # t2 = R start (row + 0)
    addi    t3, t0, 1                # t3 = G start (row + 1)
    addi    t4, t0, 2                # t4 = B start (row + 2)

    li      a4, WIDTH                # a4 = pixels remaining in this row

.Lxloop:
    vsetvli t1, a4, e8, m1, ta, ma   # t1 = VL (number of pixels this chunk)

    # Broadcast (r,g,b) across VL elements
    vmv.v.x v0, a1                   # v0 := r
    vmv.v.x v1, a2                   # v1 := g
    vmv.v.x v2, a3                   # v2 := b

    # Strided vector stores: write RGB planes with stride = 3 bytes
    vsse8.v v0, 0(t2), t6            # row[0], row[3], row[6], ...
    vsse8.v v1, 0(t3), t6            # row[1], row[4], row[7], ...
    vsse8.v v2, 0(t4), t6            # row[2], row[5], row[8], ...

    # Decrement pixels remaining in this row
    sub     a4, a4, t1

    # Compute increment = VL * 3 without using t7
    slli    a5, t1, 1                # a5 = VL * 2
    add     a5, a5, t1               # a5 = VL * 3

    # Advance channel pointers by increment
    add     t2, t2, a5
    add     t3, t3, a5
    add     t4, t4, a5

    bnez    a4, .Lxloop              # more pixels in this row?

    # Next row
    addi    t5, t5, -1
    addi    t0, t0, ROWBYTES
    bnez    t5, .Lrow

.Ldone:
    ret
    .size   fill_color_rvv, .-fill_color_rvv

