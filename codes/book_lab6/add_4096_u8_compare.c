// add_4096_u8_compare.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
int N=4096*8;  // bytes

/* --------- serial: C[i] = A[i] + B[i] (wraparound) --------- */
static void add_u8_serial(const uint8_t *restrict A,
                          const uint8_t *restrict B,
                          uint8_t *restrict C,
                          size_t n)
{
    for (size_t i = 0; i < n; ++i)
        C[i] = (uint8_t)(A[i] + B[i]);
}

/* --------- OpenMP (8 threads): same operation --------- */
static void add_u8_omp(const uint8_t *restrict A,
                       const uint8_t *restrict B,
                       uint8_t *restrict C,
                       size_t n)
{
    #pragma omp parallel for simd schedule(static) num_threads(8)
    for (size_t i = 0; i < n; ++i)
        C[i] = (uint8_t)(A[i] + B[i]);
}

/* simple checksum so the compiler can't optimize work away */
static uint64_t checksum(const uint8_t *buf, size_t n)
{
    uint64_t s = 0;
    for (size_t i = 0; i < n; ++i) s += buf[i];
    return s;
}

int main(int argc, char **argv)
{
    /* repetitions to make timing robust */
    if(argc!=2)
    {
            printf("Usage: %s vector_size\n",argv[0]);  exit(1);
    }
    N=atoi(argv[1])*1024;          // size in 1024 byte blocks
    uint8_t *A  = (uint8_t*)malloc(N);
    uint8_t *B  = (uint8_t*)malloc(N);
    uint8_t *C1 = (uint8_t*)malloc(N);
    uint8_t *C2 = (uint8_t*)malloc(N);
    if (!A || !B || !C1 || !C2) {
        fprintf(stderr, "Out of memory\n");
        free(A); free(B); free(C1); free(C2);
        return 1;
    }
    /* initialize inputs */
    for (size_t i = 0; i < N; ++i) {
        A[i] = (uint8_t)(i & 0xFF);
        B[i] = (uint8_t)((3*i + 7) & 0xFF);
    }
    /* warm-up */
    add_u8_serial(A, B, C1, N);
    add_u8_omp(A, B, C2, N);
    /* time serial (repeated) */
    double t0 = omp_get_wtime();
        add_u8_serial(A, B, C1, N);
    double t1 = omp_get_wtime();
    /* time OpenMP (repeated) */
    double t2 = omp_get_wtime();
        add_u8_omp(A, B, C2, N);
    double t3 = omp_get_wtime();
    /* verify results and show timing */
    int same = (memcmp(C1, C2, N) == 0);
        uint64_t s1 = checksum(C1, N), s2 = checksum(C2, N);
    double serial_s = t1 - t0;
    double omp_s    = t3 - t2;
    double speedup  = serial_s / (omp_s > 0 ? omp_s : 1e-9);
    printf("Serial time : %.6f s  (checksum=%llu)\n",
           serial_s, (unsigned long long)s1);
    printf("OpenMP (8)  : %.6f s  (checksum=%llu)\n",
           omp_s, (unsigned long long)s2);
    printf("Speedup     : %.2fx   (results equal: %s)\n",
           speedup, same ? "YES" : "NO");
    free(A); free(B); free(C1); free(C2);
    return same ? 0 : 2;
}

