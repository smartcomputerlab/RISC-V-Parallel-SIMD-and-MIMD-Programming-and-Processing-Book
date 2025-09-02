#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
// RVV kernel (assembly below). Float math inside, **returns double** sum_y.
// sum_y = Σ sqrtf(1 - x^2) over i in [i0, i0+count), x = (i + 0.5f) / steps
extern double integrate_quarter_circle_rvv_u64_f64acc(uint64_t i0, uint64_t count, uint64_t steps);
/* Serial midpoint rule on [0,1] for quarter circle; do math in float,
   but accumulate in double to avoid float stagnation. */
static float pi_serial_f(uint64_t steps) {
    if (steps == 0) return NAN;
    const float invN = 1.0f / (float)steps;
    double sum_y = 0.0;                 // <-- double accumulator
    for (uint64_t i = 0; i < steps; ++i) {
        float x = ((float)i + 0.5f) * invN;
        float t = 1.0f - x * x;
        if (t < 0.0f) t = 0.0f;
        sum_y += (double)sqrtf(t);      // promote each term to double
    }
    double pi = 4.0 * sum_y * (double)invN;
    return (float)pi;
}

/* OpenMP(8) + RVV kernel per thread: double reduction, float math in kernel */
static float pi_omp_rvv_f(uint64_t steps) {
    if (steps == 0) return NAN;
        double sum_y = 0.0;
    #pragma omp parallel for reduction(+:sum_y) schedule(static,1) num_threads(8)
    for (int t = 0; t < 8; ++t) {
        uint64_t i0 = (steps * (uint64_t)t)     / 8u;
        uint64_t i1 = (steps * (uint64_t)(t+1)) / 8u;
        uint64_t cnt = i1 - i0;
        if (cnt) sum_y += integrate_quarter_circle_rvv_u64_f64acc(i0, cnt, steps);
    }
    const double invN = 1.0 / (double)steps;
    double pi = 4.0 * sum_y * invN;
    return (float)pi;
}
int main(int argc, char** argv) {
    uint64_t steps = (argc > 1) ? atoi(argv[1]) : 100000000ULL;
    // Warm-up
    volatile float w1 = pi_serial_f(10000);
    volatile float w2 = pi_omp_rvv_f(10000);
    (void)w1; (void)w2;
    double t0 = omp_get_wtime();
    float pi_s = pi_serial_f(steps);
    double t1 = omp_get_wtime();
    double t2 = omp_get_wtime();
    float pi_p = pi_omp_rvv_f(steps);
    double t3 = omp_get_wtime();
    printf("steps = %llu\n", (unsigned long long)steps);
    printf("Serial   pi ≈ %.9f  time = %.6f s\n", (double)pi_s, t1 - t0);
    printf("OMP+RVV  pi ≈ %.9f  time = %.6f s\n", (double)pi_p, t3 - t2);
    printf("Combined speedup MIMD*SIMD = %.3f \n", (t1-t0)/(t3 - t2));
    printf("abs diff       = %.3e\n", fabs((double)pi_s - (double)pi_p));
    return 0;
}

