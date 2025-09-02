#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>        // compile with -fopenmp
// Big enough to showcase parallel speedup
#define WIDTH    512
#define HEIGHT   512
int  MAX_ITER = 2000;

static inline uint16_t mandelbrot_pixel(double cx, double cy) {
    double zx = 0.0, zy = 0.0;
    uint16_t iter = 0;
    while (zx*zx + zy*zy <= 4.0 && iter < MAX_ITER) {
        double zx2 = zx*zx - zy*zy + cx;
        zy = 2.0*zx*zy + cy;
        zx = zx2;
        ++iter;
    }
    return iter;
}
/* Serial renderer */
void mandelbrot_serial(uint16_t *restrict img) {
    for (int y = 0; y < HEIGHT; ++y) {
        double cy = (y / (double)HEIGHT) * 3.0 - 1.5;     // [-1.5, 1.5]
        for (int x = 0; x < WIDTH; ++x) {
            double cx = (x / (double)WIDTH) * 3.5 - 2.5;  // [-2.5, 1.0]
            img[(size_t)y * WIDTH + x] = mandelbrot_pixel(cx, cy);
        }
    }
}

/* OpenMP renderer with 8 threads */
void mandelbrot_omp(uint16_t *restrict img) {
    #pragma omp parallel for schedule(dynamic, 1) num_threads(8)
    for (int y = 0; y < HEIGHT; ++y) {
        double cy = (y / (double)HEIGHT) * 3.0 - 1.5;
        for (int x = 0; x < WIDTH; ++x) {
            double cx = (x / (double)WIDTH) * 3.5 - 2.5;
            img[(size_t)y * WIDTH + x] = mandelbrot_pixel(cx, cy);
        }
    }
}

static int compare_images(const uint16_t *a, const uint16_t *b) {
    for (size_t i = 0, n = (size_t)WIDTH * HEIGHT; i < n; ++i)
        if (a[i] != b[i]) return 0;
    return 1;
}
/* Map 16-bit iteration counts to 8-bit grayscale (inside=black) */
static void map_iters_to_u8(uint8_t *dst, const uint16_t *src) {
    size_t N = (size_t)WIDTH * HEIGHT;
    for (size_t i = 0; i < N; ++i) {
        unsigned it = src[i];
        unsigned v  = (it >= MAX_ITER) ? 0 : (255u - (it * 255u) / MAX_ITER);
        dst[i] = (uint8_t)v;
    }
}
/* Write a binary PGM (P5) */
static int write_pgm(const char *path, const uint8_t *gray, int w, int h) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror("fopen"); return 0; }
    if (fprintf(f, "P5\n%d %d\n255\n", w, h) < 0) {
        perror("fprintf"); fclose(f); return 0;
    }
    size_t N = (size_t)w * h;
    if (fwrite(gray, 1, N, f) != N) {
        perror("fwrite"); fclose(f); return 0;
    }
    fclose(f);
    return 1;
}

int main(int argc, char **argv)
{
  if(argc!=2) 
    { printf("Usage:%s number_of_iterations [64,512,1024,4096, ..] \n",argv[0]); 
      exit(1); }
  else MAX_ITER = atoi(argv[1]);

    size_t N = (size_t)WIDTH * HEIGHT;
    uint16_t *img_serial = (uint16_t*)malloc(N * sizeof(uint16_t));
    uint16_t *img_omp    = (uint16_t*)malloc(N * sizeof(uint16_t));
    uint8_t  *gray       = (uint8_t*) malloc(N);
    if (!img_serial || !img_omp || !gray) {
        fprintf(stderr, "Out of memory\n");
        free(img_serial); free(img_omp); free(gray);
        return 1;
    }
    // Warm-up (optional for fair timing)
    mandelbrot_serial(img_serial);
    double t0 = omp_get_wtime();
    mandelbrot_serial(img_serial);
    double t1 = omp_get_wtime();
    double t2 = omp_get_wtime();
    mandelbrot_omp(img_omp);
    double t3 = omp_get_wtime();
    int ok_same = compare_images(img_serial, img_omp);
    printf("Serial time : %.3f s\n", t1 - t0);
    printf("OpenMP(8)   : %.3f s\n", t3 - t2);
    printf("Speedup      : %.2fx\n", (t1 - t0) / (t3 - t2));
    printf("Images equal : %s\n", ok_same ? "YES" : "NO");
    map_iters_to_u8(gray, img_omp);
    if (!write_pgm("mandelbrot_omp.pgm", gray, WIDTH, HEIGHT)) {
        fprintf(stderr, "Failed to write PGM\n");
    } else {
        printf("Wrote mandelbrot_omp.pgm (%dx%d)\n", WIDTH, HEIGHT);
    }
    free(img_serial);
    free(img_omp);
    free(gray);
    return ok_same ? 0 : 2;
}

