// pi_rectangles.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <time.h>
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
// Assembly function (from pi_rectangles_rvv.S)
extern double pi_rectangles_rvv(size_t N);
// Scalar reference (same midpoint rule)
static double pi_rectangles_scalar(size_t N) {
    if (N == 0) return 0;
    const double dx = 1.0 / (double)N;
    double sum = 0.0;
    for (size_t i = 0; i < N; ++i) {
        const double x = (i + 0.5) * dx;
        sum += sqrt(1.0 - x * x);
    }
    return 4.0 * dx * sum;
}

int main(int argc, char** argv) {
    size_t N = 4096*16;                    // default as in your example
    if (argc == 2) {
            N=atoi(argv[1]);
    }
    startv=clock();
    // Call the assembly function
    double pi_vec = pi_rectangles_rvv(N);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    start=clock();
    // Call the C function
    double pi_ref = pi_rectangles_scalar(N);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    double pi_true = acos(-1.0);
    printf("N = %zu\n", N);
    printf("RVV   π ≈ %.12f\n", pi_vec);
    printf("Scalar π ≈ %.12f\n", pi_ref);
    printf("True   π = %.12f\n", pi_true);
    if (!isnan(pi_vec)) {
        printf("Error (RVV  ) = %.6e\n", pi_vec - pi_true);
    } else {  printf("RVV result is NaN (likely N=0)\n"); }
    if (!isnan(pi_ref)) {
        printf("Error (Scalar) = %.6e\n", pi_ref - pi_true);
    } else {
        printf("Scalar result is NaN (likely N=0)\n");
    }
    return 0;
}

