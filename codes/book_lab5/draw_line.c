#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define WIDTH   512
#define HEIGHT  512
clock_t startv, endv;
clock_t start, end;
double elapsed_secv;
double elapsed_sec;
int steps=100000;

// Assembly function (RV64GCV+Zbb) you already have:
extern "C" void draw_line_rvv(uint8_t *image,
                      int x0, int y0,
                      int x1, int y1,
                      uint8_t value);

/* Unbiased random integer in [0, n-1] using rejection sampling */
static unsigned rand_bounded(unsigned n) {
    unsigned limit = RAND_MAX - (RAND_MAX % n);  // largest multiple of n ≤ RAND_MAX
    unsigned r;
    do {
        r = (unsigned)rand();
    } while (r >= limit);
    return r % n;
}
// Function to draw a line between two points using Bresenham's algorithm
void draw_line(uint8_t *image, int x0, int y0, int x1, int y1, uint8_t value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while (1) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            image[y0 * WIDTH + x0] = value;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
// Write an 8-bit grayscale PGM (binary P5)
static int write_pgm(const char *path, const uint8_t *img, int w, int h)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        perror("fopen");
        return -1;
    }
    // P5 header: magic, width height, maxval
    if (fprintf(f, "P5\n%d %d\n255\n", w, h) < 0) {
        perror("fprintf");
        fclose(f);
        return -1;
    }
    size_t n = (size_t)w * (size_t)h;
    if (fwrite(img, 1, n, f) != n) {
        perror("fwrite");
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

// Optional helper to clear image
static void clear_image(uint8_t *img, uint8_t value)
{
    memset(img, value, (size_t)WIDTH * (size_t)HEIGHT);
}
int main(void)
{
    // Allocate framebuffer
    uint8_t *image = (uint8_t*)malloc((size_t)WIDTH * (size_t)HEIGHT);
    if (!image) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }
    int x0=rand_bounded(512);
    int x1=rand_bounded(512);
    int y0=rand_bounded(512);
    int y1=rand_bounded(512);
    // Border box
    clear_image(image, 0);
    startv=clock();
    for(int i=0;i<steps;i++) {
    draw_line_rvv(image, x0, x1, y0, y1, 255);
    }
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    // Border box
    clear_image(image, 0);
    start=clock();
    for(int i=0;i<steps;i++) {
    draw_line(image, x0, x1, y0, y1, 255);
    }
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Save as PGM
    const char *out_path = "output.pgm";
    if (write_pgm(out_path, image, WIDTH, HEIGHT) != 0) {
        fprintf(stderr, "Failed to write %s\n", out_path);
        free(image);
        return 1;
    }
    printf("Wrote %s (%dx%d)\n", out_path, WIDTH, HEIGHT);
    free(image);
    return 0;
}

