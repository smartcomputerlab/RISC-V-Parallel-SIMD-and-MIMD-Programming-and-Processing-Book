.text
.align 2
.option push
.option arch, +v
.globl create_square_rvv

# void create_square_rvv(uint8_t* image, int center_x, int center_y, int size, uint8_t value)
# a0 = image, a1 = center_x, a2 = center_y, a3 = size, a4 = value
create_square_rvv:
    # Early return if size <= 0
    blez    a3, .Lret

    # Save registers
    addi    sp, sp, -32
    sd      s0, 0(sp)
    sd      s1, 8(sp)
    sd      s2, 16(sp)
    sd      s3, 24(sp)

    # Calculate top-left corner: start_x = center_x - size/2
    srli    t0, a3, 1                  # t0 = size / 2
    sub     t1, a1, t0                 # t1 = start_x = center_x - size/2
    sub     t2, a2, t0                 # t2 = start_y = center_y - size/2

    # Clamp start coordinates to image bounds (0 <= start_x, start_y <= 511)
    li      t3, 0                      # t3 = 0 (min bound)
    li      t4, 512                    # t4 = 512 (max bound)
    
    # Clamp start_x to [0, 511]
    blt     t1, t3, .Lclamp_x_min
    bge     t1, t4, .Lclamp_x_max
    j       .Lclamp_x_done
.Lclamp_x_min:
    li      t1, 0
    j       .Lclamp_x_done
.Lclamp_x_max:
    li      t1, 511
.Lclamp_x_done:

    # Clamp start_y to [0, 511]
    blt     t2, t3, .Lclamp_y_min
    bge     t2, t4, .Lclamp_y_max
    j       .Lclamp_y_done
.Lclamp_y_min:
    li      t2, 0
    j       .Lclamp_y_done
.Lclamp_y_max:
    li      t2, 511
.Lclamp_y_done:

    # Calculate end coordinates
    add     t5, t1, a3                 # t5 = end_x = start_x + size
    add     t6, t2, a3                 # t6 = end_y = start_y + size
    
    # Clamp end coordinates to [0, 512]
    blt     t5, t3, .Lclamp_ex_min
    bge     t5, t4, .Lclamp_ex_max
    j       .Lclamp_ex_done
.Lclamp_ex_min:
    li      t5, 0
    j       .Lclamp_ex_done
.Lclamp_ex_max:
    li      t5, 512
.Lclamp_ex_done:

    blt     t6, t3, .Lclamp_ey_min
    bge     t6, t4, .Lclamp_ey_max
    j       .Lclamp_ey_done
.Lclamp_ey_min:
    li      t6, 0
    j       .Lclamp_ey_done
.Lclamp_ey_max:
    li      t6, 512
.Lclamp_ey_done:

    # Calculate actual width and height after clamping
    sub     s0, t5, t1                 # s0 = actual_width
    sub     s1, t6, t2                 # s1 = actual_height
    
    # Early return if no area to fill
    blez    s0, .Lcleanup
    blez    s1, .Lcleanup

    # Calculate image row stride (512 bytes)
    li      s2, 512                    # s2 = image width (stride)
    
    # Calculate starting address in image buffer
    mul     t3, t2, s2                 # t3 = start_y * stride
    add     t3, t3, t1                 # t3 += start_x
    add     s3, a0, t3                 # s3 = image + start_y*stride + start_x
    
    # Broadcast the fill value to a vector register
    vsetvli t4, x0, e8, m1, ta, ma     # Set SEW=8, no elements yet
    vmv.v.x v1, a4                     # v1 = broadcast(value)
    
    # Loop through each row
    li      t0, 0                      # t0 = row counter
.Lrow_loop:
    bge     t0, s1, .Lcleanup          # Exit when all rows processed
    
    # Calculate current row address
    mul     t1, t0, s2                 # t1 = row_offset = row * stride
    add     t2, s3, t1                 # t2 = current_row_address
    
    # Fill the row using vector instructions
    mv      t3, s0                     # t3 = remaining width
    mv      t4, t2                     # t4 = current address
    
.Lfill_row:
    # Set vector length for this chunk
    vsetvli t5, t3, e8, m1, ta, ma     # t5 = min(remaining, VLMAX)
    
    # Store vector to memory
    vse8.v  v1, (t4)
    
    # Advance pointers
    add     t4, t4, t5                 # Move destination pointer
    sub     t3, t3, t5                 # Decrement remaining count
    bnez    t3, .Lfill_row             # Continue if more to fill
    
    # Next row
    addi    t0, t0, 1                  # row_counter++
    j       .Lrow_loop

.Lcleanup:
    # Restore registers
    ld      s0, 0(sp)
    ld      s1, 8(sp)
    ld      s2, 16(sp)
    ld      s3, 24(sp)
    addi    sp, sp, 32

.Lret:
    ret
.option pop

