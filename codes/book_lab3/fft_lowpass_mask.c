#include <stdio.h>
#include <stdlib.h>
#include <time.h>
clock_t startv, endv;
clock_t start, end;
double elapsed_secv;
double elapsed_sec;

/* Prototype of the assembly function */
extern void fft_lowpass_mask_f32_rvv(float* Xa, int N, int k_cut);

void fft_lowpass_mask_f32(float* Xc, int N, int k_cut)
{
    if (N <= 0 || k_cut < 0) return;
    if (k_cut > N/2) k_cut = N/2;
    for (int k = 0; k < N; ++k) {
        /* circular (wrap-around) distance to DC (bin 0) */
        int d = k;
        int alt = N - k;
        if (alt < d) d = alt;          /* d = min(k, N-k) */
        if (d > k_cut) {
            Xc[2*k + 0] = 0.0f;         /* real */
            Xc[2*k + 1] = 0.0f;         /* imag */
        }
        /* else: keep Xc[k] as is */
    }
}

int steps=1000000;

int main(void) {
    const int N = 16;           // number of complex bins
    const int k_cut = 3;        // keep only bins with |k| <= 3
    float Xc[2*N];               // interleaved complex array: [Re0,Im0, Re1,Im1, ...]
    float Xa[2*N];               // interleaved complex array: [Re0,Im0, Re1,Im1, ...]
    // Fill with simple test data: Re[k] = k, Im[k] = -k
    for (int k = 0; k < N; ++k) {
        Xc[2*k+0] = (float)k;     // real
        Xc[2*k+1] = (float)(-k);  // imag
    }
    printf("Before mask:\n");
    for (int k = 0; k < N; ++k) {
        printf("k=%2d: (%6.2f, %6.2f)\n", k, Xc[2*k], Xc[2*k+1]);
    }
    // Call the RVV C filter
    start=clock();
    for(int i=0; i<steps;i++) fft_lowpass_mask_f32(Xc, N, k_cut);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("\nAfter low-pass mask (k_cut=%d):\n", k_cut);
    for (int k = 0; k < N; ++k) {
        printf("k=%2d: (%6.2f, %6.2f)\n", k, Xc[2*k], Xc[2*k+1]);
    }
    printf("\n\n");
    // Fill with simple test data: Re[k] = k, Im[k] = -k
    for (int k = 0; k < N; ++k) {
        Xa[2*k+0] = (float)k;     // real
        Xa[2*k+1] = (float)(-k);  // imag
    }
    printf("Before mask:\n");
    for (int k = 0; k < N; ++k) {
        printf("k=%2d: (%6.2f, %6.2f)\n", k, Xa[2*k], Xa[2*k+1]);
    }
    // Call the RVV assembly filter
    startv=clock();
    for(int i=0;i<steps;i++) fft_lowpass_mask_f32_rvv(Xa, N, k_cut);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    printf("\nAfter low-pass mask (k_cut=%d):\n", k_cut);
    for (int k = 0; k < N; ++k) {
        printf("k=%2d: (%6.2f, %6.2f)\n", k, Xa[2*k], Xa[2*k+1]);
    }
    return 0;
}

