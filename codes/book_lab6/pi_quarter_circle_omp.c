#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>   // build with -fopenmp

// Integrand for quarter circle of radius 1
static inline double quarter_circle(double x) {
    return sqrt(1.0 - x*x);
}

// Serial rectangle (midpoint) rule on [0,1]; PI = 4 * integral_0^1 sqrt(1-x^2) dx
double pi_serial(uint64_t steps) {
    if (steps == 0) return NAN;
    const double invN = 1.0 / (double)steps;
    double sum = 0.0;
    for (uint64_t i = 0; i < steps; ++i) {
        double x = ( (double)i + 0.5 ) * invN;   // midpoint
        sum += quarter_circle(x);
    }
    return 4.0 * sum * invN;
}

// OpenMP version: same algorithm, parallelized across 8 threads
double pi_omp(uint64_t steps) {
    if (steps == 0) return NAN;
    const double invN = 1.0 / (double)steps;
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static) num_threads(8)
    for (uint64_t i = 0; i < steps; ++i) {
        double x = ( (double)i + 0.5 ) * invN;   // midpoint
        sum += quarter_circle(x);
    }
    return 4.0 * sum * invN;
}

int main(int argc, char **argv) {
    // Number of rectangles (steps) — can be large; adjust on the command line
    uint64_t steps = (argc > 1) ? strtoull(argv[1], NULL, 10) : (uint64_t)100000000; // 1e8 by default
    // Warm-up (helps make timings fair on some systems)
    volatile double w1 = pi_serial(10000);
    volatile double w2 = pi_omp(10000);
    (void)w1; (void)w2;
    // Serial timing
    double t0 = omp_get_wtime();
    double pi_s = pi_serial(steps);
    double t1 = omp_get_wtime();

    // OpenMP timing (8 threads)
    double t2 = omp_get_wtime();
    double pi_p = pi_omp(steps);
    double t3 = omp_get_wtime();
    double ts = t1 - t0;
    double tp = t3 - t2;
    double speedup = ts / (tp > 0.0 ? tp : 1e-12);
    // Report
    printf("Rectangles (steps): %llu\n", (unsigned long long)steps);
    printf("Serial  pi ≈ %.12f  time = %.6f s\n", pi_s, ts);
    printf("OpenMP8 pi ≈ %.12f  time = %.6f s\n", pi_p, tp);
    printf("Speedup: %.2fx   |  |Δ| = %.12e\n", speedup, fabs(pi_s - pi_p));
    return 0;
}

