# rgb_neg_rvv.s
# void negate_buffer_rvv(const uint8_t* inbuf, uint8_t* outbuf,
#                    int width, int height, int channels);
# out[i] = 255 - in[i] for i in [0 .. width*height*channels)

    .text
    .globl  negate_buffer_rvv
    .type   negate_buffer_rvv, @function
negate_buffer_rvv:
    # a0 = inbuf (u8*)
    # a1 = outbuf (u8*)
    # a2 = width (int)
    # a3 = height (int)
    # a4 = channels (int)
    # --- compute total element count: size_t size = width * height * channels ---
    # Use 64-bit math (RV64). If building RV32, keep values small enough or widen manually.
    sext.w   a5, a2             # a5 = (int64)width
    sext.w   t0, a3             # t0 = (int64)height
    mul      a5, a5, t0         # a5 = width * height
    sext.w   t1, a4             # t1 = (int64)channels
    mul      a5, a5, t1         # a5 = width*height*channels
    beqz     a5, .Ldone         # nothing to do if size == 0
    li       t3, 255

.Lloop:
    # Set VL for remaining bytes (SEW=8, LMUL=1). Tail-agnostic, mask-agnostic.
    vsetvli  t2, a5, e8, m1, ta, ma   # t2 = vl (elements this chunk)
    # Load 'vl' bytes from input
    vle8.v   v0, (a0)
    # v1 = 255 - v0   (reverse subtract with immediate)
    vrsub.vx v1, v0, t3
    # Store 'vl' bytes to output
    vse8.v   v1, (a1)
    # Advance pointers by vl bytes, decrement remaining by vl elements
    add      a0, a0, t2
    add      a1, a1, t2
    sub      a5, a5, t2
    bnez     a5, .Lloop

.Ldone:
    ret
    .size negate_buffer_rvv, .-negate_buffer_rvv

