# bgr_gray_rvv.s
# Convert interleaved BGR -> GRAY (u8), Y = ((29*B + 150*G + 77*R + 128) >> 8).
# a0 = bgr (u8*), a1 = gray (u8*), a2 = pixels (count of BGR triplets)

    .text
    .globl  bgr_gray_rvv
    .align  2
bgr_gray_rvv:
    beqz    a2, 2f                      # nothing to do

    li      t3, 29                      # coefB
    li      t4, 150                     # coefG
    li      t5, 77                      # coefR

1:
    # Decide VL for compute phase (e16), and reuse it for load/store
    vsetvli t0, a2, e16, m1, ta, ma     # t0 = vl
    # --- Load B,G,R bytes (use the SAME vl) ---
    vsetvli x0, t0, e8,  m1, ta, ma
    vlseg3e8.v  v0, (a0)                # v0=B (e8), v1=G, v2=R
    # --- Widen to u16 and compute sum in 16-bit lanes ---
    vsetvli x0, t0, e16, m1, ta, ma
    vzext.vf2  v8,  v0                  # B -> u16
    vzext.vf2  v9,  v1                  # G -> u16
    vzext.vf2  v10, v2                  # R -> u16
    vmul.vx    v11, v8,  t3             # 29*B
    vmul.vx    v12, v9,  t4             # 150*G
    vmul.vx    v13, v10, t5             # 77*R
    vadd.vv    v14, v11, v12
    vadd.vv    v14, v14, v13            # v14 = sum (0..65408), u16
    # --- Narrow with rounding: (sum + 128) >> 8  -> e8 lanes (no aliasing) ---
    vsetvli x0, t0, e8,  m1, ta, ma
    vnclipu.wi v16, v14, 8              # v16 = u8 result (rounded)
    # --- Store bytes (same vl) ---
    vse8.v     v16, (a1)
    # Advance pointers: bgr += 3*vl, gray += vl, pixels -= vl
    slli       t1,  t0, 1               # t1 = 2*vl
    add        t1,  t1, t0              # t1 = 3*vl
    add        a0,  a0, t1
    add        a1,  a1, t0
    sub        a2,  a2, t0
    bnez       a2, 1b

2:  ret

