# void draw_line(uint8_t *image, int x0, int y0, int x1, int y1, uint8_t value)
# a0=image, a1=x0, a2=y0, a3=x1, a4=y1, a5=value
# Requires: RV64GCV + Zbb

    .text
    .align 2
    .globl draw_line_rvv
    .type  draw_line_rvv, @function
    .equ WIDTH,  512
    .equ HEIGHT, 512

draw_line_rvv:
    addi    sp, sp, -112
    sd      ra,   0(sp)
    sd      s0,   8(sp)
    sd      s1,  16(sp)
    sd      s2,  24(sp)
    sd      s3,  32(sp)
    sd      s4,  40(sp)
    sd      s5,  48(sp)
    sd      s6,  56(sp)
    sd      s7,  64(sp)
    sd      s8,  72(sp)
    sd      s9,  80(sp)
    sd      s10, 88(sp)
    sd      s11, 96(sp)

    mv      s0, a0        # image*
    mv      s1, a1        # x0
    mv      s2, a2        # y0
    mv      s3, a3        # x1
    mv      s4, a4        # y1
    mv      s5, a5        # value (byte)

# dx = abs(x1 - x0)
    sub     t0, s3, s1
    neg     t1, t0
    max     s6, t0, t1    # dx
# dy = abs(y1 - y0)
    sub     t0, s4, s2
    neg     t1, t0
    max     s7, t0, t1    # dy
# sx = (x0 < x1) ? 1 : -1
    li      t2, 1
    li      t3, -1
    blt     s1, s3, 1f
    mv      s8, t3
    j       2f
1:  mv      s8, t2
2:
# sy = (y0 < y1) ? 1 : -1
    blt     s2, s4, 3f
    mv      s9, t3
    j       4f
3:  mv      s9, t2
4:
# err = dx - dy
    sub     s10, s6, s7

# --- Fast paths
    beqz    s7, .HORIZ_PATH          # dy == 0
    beqz    s6, .VERT_PATH           # dx == 0
    j       .BRESENHAM

# -------------------------------------------------------------------
# Horizontal line
# -------------------------------------------------------------------
.HORIZ_PATH:
    bltz    s2, .DONE
    li      t4, HEIGHT
    bgeu    s2, t4, .DONE
    min     t5, s1, s3              # x_start
    max     t6, s1, s3              # x_end
    max     t5, t5, x0              # clamp start to 0
    li      t1, WIDTH-1             # (replaces former t7)
    minu    t6, t6, t1              # clamp end to WIDTH-1
    bgt     t5, t6, .DONE
    li      t0, WIDTH
    mul     t1, s2, t0              # y*WIDTH
    add     t1, t1, t5              # + x_start
    add     t1, s0, t1              # start ptr
    sub     t2, t6, t5
    addi    t2, t2, 1               # len

.HORIZ_LOOP:
    beqz    t2, .DONE
    vsetvli t3, t2, e8,m1,ta,ma
    vmv.v.x v1, s5
    vse8.v  v1, (t1)
    csrr    t4, vl
    add     t1, t1, t4
    sub     t2, t2, t4
    j       .HORIZ_LOOP

# -------------------------------------------------------------------
# Vertical line
# -------------------------------------------------------------------
.VERT_PATH:
    bltz    s1, .DONE
    li      t4, WIDTH
    bgeu    s1, t4, .DONE
    min     t5, s2, s4              # y_start
    max     t6, s2, s4              # y_end
    max     t5, t5, x0              # clamp start to 0
    li      t1, HEIGHT-1            # (replaces former t7)
    minu    t6, t6, t1              # clamp end to HEIGHT-1
    bgt     t5, t6, .DONE
    li      t0, WIDTH
    mul     t1, t5, t0
    add     t1, t1, s1
    add     t1, s0, t1              # base
    sub     t2, t6, t5
    addi    t2, t2, 1               # len
    mv      t5, t0                  # stride = WIDTH

.VERT_LOOP:
    beqz    t2, .DONE
    vsetvli t3, t2, e8,m1,ta,ma
    vmv.v.x v1, s5
    vsse8.v v1, (t1), t5
    csrr    t4, vl
    mul     t4, t4, t5              # advance = vl * stride
    add     t1, t1, t4
    sub     t2, t2, t3
    j       .VERT_LOOP
# -------------------------------------------------------------------
# General Bresenham (scalar)
# -------------------------------------------------------------------
.BRESENHAM:
.BRESE_LOOP:
    bltz    s1, .SKIP_PX
    bltz    s2, .SKIP_PX
    li      t0, WIDTH
    bgeu    s1, t0, .SKIP_PX
    li      t1, HEIGHT
    bgeu    s2, t1, .SKIP_PX
    mul     t2, s2, t0              # y*WIDTH
    add     t2, t2, s1              # + x
    add     t2, s0, t2              # ptr
    sb      s5, 0(t2)
.SKIP_PX:
    beq     s1, s3, 1f
    bne     s2, s4, 2f
1:  beq     s2, s4, .DONE

2:  slli    t3, s10, 1              # e2 = 2*err
    neg     t4, s7
    ble     t3, t4, 3f              # if (e2 > -dy)
    sub     s10, s10, s7
    add     s1, s1, s8
3:
    bge     t3, s6, 4f              # if (e2 < dx)
    add     s10, s10, s6
    add     s2, s2, s9
4:  j       .BRESE_LOOP
# -------------------------------------------------------------------
.DONE:
    ld      ra,   0(sp)
    ld      s0,   8(sp)
    ld      s1,  16(sp)
    ld      s2,  24(sp)
    ld      s3,  32(sp)
    ld      s4,  40(sp)
    ld      s5,  48(sp)
    ld      s6,  56(sp)
    ld      s7,  64(sp)
    ld      s8,  72(sp)
    ld      s9,  80(sp)
    ld      s10, 88(sp)
    ld      s11, 96(sp)
    addi    sp, sp, 112
    ret

