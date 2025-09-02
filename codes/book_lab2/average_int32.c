//  average_int32.c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// Assembly function prototype
extern int32_t avg_i32_rvv(const int32_t *src, size_t N);

int steps=10000;

int32_t  avg(const int32_t *src, int MAX)
{
    int64_t sum = 0;
    for (int i = 0; i < MAX; i++) sum += src[i];
    return (int32_t) sum/MAX;
}

int main(void) {
    enum { N = 1024 };
    int32_t data[N];
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    int32_t avg_vec, avg_c;

    // Fill test data with a simple pattern
    for (int i = 0; i < N; i++) {
        data[i] = i;  // (i % 50) - 25; // values from -25 to 24
    }

    // Call the RVV assembly function
    startv=clock();
    for(int i=0;i<steps;i++) avg_vec = avg_i32_rvv(data, 1024);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);

    // Call the scalar function
    start=clock();
    for(int i=0;i<steps;i++) avg_c=avg(data,N);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);

    // Display results
    printf("Vector execution result: %d\n", avg_vec);
    printf("Scalar execution result: %d\n", avg_c);

    // Optional: check if they match
    if (avg_vec == avg_c) {
        printf("Test PASSED\n");
        return 0;
    } else {
        printf("Test FAILED\n");
        return 1;
    }
}

