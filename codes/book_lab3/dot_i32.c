#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

int N=0;
// Declare the RISC-V vector assembly function
int32_t dot_i32_rvv(const int32_t *a, const int32_t *b, size_t n);

    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    int steps=1000;
    int VL=32;    // MAX = N
		  //
int32_t dot_i32(const int32_t *a, const int32_t *b, size_t n)
    {
        int32_t sum=0;
        for(int32_t i=0;i<n;i++) sum= a[i]*b[i]+sum;
        return sum;
    }

int main(int argc, char **argv) {
    if(argc!=2) { printf("Usage: %s  vector_size \n",argv[0]); exit(1); }
    else N=atoi(argv[1]);
    int32_t *a=malloc(N*4);
    int32_t *b=malloc(N*4);
    for(int32_t i=0;i<N;i++)
    {
        a[i]=1;b[i]=2;
    }
    int32_t result=0;
    start=clock();
    for(int i=0;i<steps;i++) result = dot_i32(a, b, N);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Scalar dot product = %d\n", result);
    int32_t result_rvv=0;
    startv=clock();
    for(int i=0;i<steps;i++) result_rvv = dot_i32_rvv(a, b, N);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Vector dot product = %d\n", result_rvv);
    printf("Speed-up: %0.3f\n", elapsed_sec/elapsed_secv);
    return 0;
}

