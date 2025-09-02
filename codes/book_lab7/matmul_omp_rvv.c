// matmul_omp_rvv.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// RVV kernel: dot = sum_{k=0..n-1} a[k] * b[k*stride_bytes/4]
// a is unit-stride; b is strided by stride_bytes (e.g., N*sizeof(float))
int N=100;
//
extern float dot_f32_rvv_strided(const float *a,
                                 const float *b,
                                 uint64_t n,
                                 uint64_t stride_bytes);

/* Serial triple-loop baseline (float) */
static void matmul_serial(const float *A, const float *B, float *C) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float acc = 0.0f;
            for (int k = 0; k < N; ++k) {
                acc += A[i*(size_t)N + k] * B[k*(size_t)N + j];
            }
            C[i*(size_t)N + j] = acc;
        }
    }
}

/* OpenMP outer loops + RVV inner dot-product (row of A · column of B) */
static void matmul_omp_rvv(const float *A, const float *B, float *C) {
    const uint64_t n = N;
    const uint64_t strideB = (uint64_t)N * sizeof(float); // bytes between successive elements in a column
    #pragma omp parallel for collapse(2) schedule(static) num_threads(8)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            const float *rowA = &A[i*(size_t)N];
            const float *colB = &B[j];               // start of column j
            float dot = dot_f32_rvv_strided(rowA, colB, n, strideB);
            C[i*(size_t)N + j] = dot;
        }
    }
}
static double max_abs_diff(const float *X, const float *Y) {
    double m = 0.0;
    for (size_t i = 0; i < (size_t)N*N; ++i) {
        double d = fabs((double)X[i] - (double)Y[i]);
        if (d > m) m = d;
    }
    return m;
}

int main(int argc, char **argv) {
    if(argc!=2) { printf("Usage: %s  matrix_size\n",argv[0]); exit(1); }
    else N=atoi(argv[1]);
    float *A = (float*)aligned_alloc(64, N*N*sizeof(float));
    float *B = (float*)aligned_alloc(64, N*N*sizeof(float));
    float *C_ser = (float*)aligned_alloc(64, N*N*sizeof(float));
    float *C_omp = (float*)aligned_alloc(64, N*N*sizeof(float));
    if (!A || !B || !C_ser || !C_omp) {
        fprintf(stderr, "OOM\n");
        return 1;
    }
    // Init with deterministic values
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < N; ++k)
            A[i*(size_t)N + k] = 0.001f*(float)(i + 1) + 0.002f*(float)(k + 1);
    for (int k = 0; k < N; ++k)
        for (int j = 0; j < N; ++j)
            B[k*(size_t)N + j] = 0.003f*(float)(k + 1) - 0.004f*(float)(j + 1);
    // Warm-up
    matmul_serial(A, B, C_ser);
    matmul_omp_rvv(A, B, C_omp);

    // Time serial
    double t0 = omp_get_wtime();
    matmul_serial(A, B, C_ser);
    double t1 = omp_get_wtime();
    // Time OMP + RVV
    double t2 = omp_get_wtime();
    matmul_omp_rvv(A, B, C_omp);
    double t3 = omp_get_wtime();
    double diff = max_abs_diff(C_ser, C_omp);
        printf("N=%d\n", N);
    printf("Serial   : %.6f s\n", t1 - t0);
    printf("OMP+RVV  : %.6f s\n", t3 - t2);
    printf("Speedup  : %.2fx\n", (t1 - t0)/((t3 - t2) > 0.0 ? (t3 - t2) : 1e-9));
    printf("Max |Δ|  : %.3e\n", diff);
    free(A); free(B); free(C_ser); free(C_omp);
    return 0;
}

