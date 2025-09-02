// pi_all.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <stdint.h>

static double quarter_sum_scalar(size_t i0, size_t count, size_t steps) {
    const double invN = 1.0 / (double)steps;
    double sum = 0.0;
    for (size_t i = 0; i < count; ++i) {
        double x = ((double)(i0 + i) + 0.5) * invN;
        double t = 1.0 - x*x;
        if (t < 0.0) t = 0.0;
        sum += sqrt(t);
    }
    return sum;
}

static double pi_serial(size_t steps) {
    double s = quarter_sum_scalar(0, steps, steps);
    return 4.0 * s / (double)steps;
}

static double pi_omp8(size_t steps) {
    const int T = 8;
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum) schedule(static) num_threads(8)
    for (int t = 0; t < T; ++t) {
        size_t i0 = (steps * (size_t)t)     / (size_t)T;
        size_t i1 = (steps * (size_t)(t+1)) / (size_t)T;
        size_t cnt = i1 - i0;
        if (cnt) sum += quarter_sum_scalar(i0, cnt, steps);
    }
    return 4.0 * sum / (double)steps;
}

extern double integrate_quarter_circle_rvv_u64(size_t i0, size_t count, size_t steps);

static double pi_rvv(size_t steps) {
    double s = integrate_quarter_circle_rvv_u64(0, steps, steps);
    return 4.0 * s / (double)steps;
}
int main(int argc, char** argv) {
    int steps=0;	
    if(argc!=2) { printf("Usage: %s number_of_steps \n",argv[0]); exit(1);}
    else  steps=atoi(argv[1]);
    (void)pi_serial(10000);
    (void)pi_omp8(10000);
    double t0 = omp_get_wtime();
    double pis = pi_serial(steps);
    double t1 = omp_get_wtime();
    double t2 = omp_get_wtime();
    double pio = pi_omp8(steps);
    double t3 = omp_get_wtime();
    double t4 = omp_get_wtime();
    double piv = pi_rvv(steps);   // require V extension CPU/QEMU
    double t5 = omp_get_wtime();
    printf("steps = %llu\n", (unsigned long long)steps);
    printf("Serial   pi ≈ %.12f   time = %.6f s\n", pis, t1 - t0);
    printf("OpenMP8  pi ≈ %.12f   time = %.6f s   speedup: %.2fx\n",
           pio, t3 - t2, (t1 - t0)/((t3 - t2) ? (t3 - t2) : 1e-9));
    printf("RVV asm  pi ≈ %.12f   time = %.6f s   speedup: %.2fx\n",
           piv, t5 - t4, (t1 - t0)/((t5 - t4) ? (t5 - t4) : 1e-9));
    printf("abs diff OMP-Serial = %.3e, RVV-Serial = %.3e\n",
           fabs(pio - pis), fabs(piv - pis));
    return 0;
}

