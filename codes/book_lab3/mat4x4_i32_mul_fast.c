#include <stdio.h>
#include <stdint.h>
#include <time.h>

// Declare the external assembly function
extern void mat4x4_i32_mul_fast_rvv(const int32_t *A, const int32_t *B, int32_t *C);

// Multiply two 4×4 signed byte matrices (row-major)
// A[4][4], B[4][4] → C[4][4] (int32_t)
void mat4x4_i32_mul_c(const int32_t *A, const int32_t *B, int32_t *C) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int32_t sum = 0;
            for (int k = 0; k < 4; k++) {
                // A[i,k] * B[k,j]
                sum += (int32_t)A[i*4 + k] * (int32_t)B[k*4 + j];
            }
            C[i*4 + j] = sum;
        }
    }
}

static void print_matrix_i32(const int32_t *M) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%6d ", M[i*4 + j]);
        }
        printf("\n");
    }
}

int main(void) {
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    // Example 4×4 int8_t matrices (row-major)
    int32_t A[16] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16
    };

    int32_t B[16] = {
        -1,  0,  1,  2,
         3, -2,  0,  1,
         4,  5, -3,  0,
         2,  1,  0, -1
    };

    int32_t CA[16] = {0}; // Output
    int32_t CC[16] = {0}; // Output
    // Print result
    printf("Matrix A:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%4d ", A[i*4 + j]);
        printf("\n");
    }

    printf("\nMatrix B:\n");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            printf("%4d ", B[i*4 + j]);
        printf("\n");
    }
    int steps=1000000;    // 1 million
    startv=clock();
    // Call the assembly function
    for(int i=0;i<steps;i++) mat4x4_i32_mul_fast_rvv(A, B, CA);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("\nResult matrix C in assembly with vector extension (int32_t):\n");
    print_matrix_i32(CA);
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    start=clock();
    // Call the C function
    for(int i=0;i<steps;i++) mat4x4_i32_mul_c(A, B, CC);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\nResult matrix C in C (int32_t):\n");
    print_matrix_i32(CC);
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    return 0;
}

