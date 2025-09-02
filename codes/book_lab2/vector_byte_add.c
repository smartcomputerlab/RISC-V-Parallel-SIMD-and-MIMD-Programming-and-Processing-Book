// vector_byte_add.c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
// external assembly function 
extern void vector_byte_add_rvv(const int8_t *a, const int8_t *b, int8_t *c, size_t n);

int main(void) {
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    const int N = 32;
    int8_t a[N], b[N], c[N];
    int steps=1000000;
    // Initialize test vectors
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
    }
    startv=clock();
    // Call vector addition
    for(int i=0;i<steps;i++) vector_byte_add_rvv(a, b, c, N);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    start=clock();
    for(int i=0;i<steps;i++)
    { for( int j=0;j<32;j++) c[j] = a[j] + b[j];}
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    // Print results
    printf("Addition result:\n");
    for (int i = 0; i < N; i++) {
        printf("%d + %d = %d\n", a[i], b[i], c[i]);
    }
    printf("Vector execution time: %.6f sec\n",elapsed_secv);
    printf("Scalar execution time: %.6f sec\n",elapsed_sec);
    printf("Speed-up: %.3f sec\n",elapsed_sec/elapsed_secv);
    return 0;
}

