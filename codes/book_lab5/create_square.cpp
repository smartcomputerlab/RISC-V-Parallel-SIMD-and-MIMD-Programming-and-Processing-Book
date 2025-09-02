#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define WIDTH 512
#define HEIGHT 512
#include <time.h>

    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    int steps=100000;
// Assembly function prototype
extern void create_square_rvv(uint8_t* image, int center_x, int center_y, int size, uint8_t value);
// Function to save image as PGM
void save_pgm(const char *filename, uint8_t *image, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("Error opening file %s\n", filename);
        return;
    }
    fprintf(fp, "P5\n%d %d\n255\n", width, height);
    size_t written = fwrite(image, 1, width * height, fp);
    fclose(fp);
    printf("Image saved as %s (%zu bytes written)\n", filename, written);
}

// Simple scalar version for comparison
void create_square_scalar(uint8_t *image, int center_x, int center_y, int size, uint8_t value) {
    if (size <= 0) return;
    int start_x = center_x - size/2;
    int start_y = center_y - size/2;
    // Clamp coordinates
    start_x = (start_x < 0) ? 0 : (start_x >= WIDTH) ? WIDTH-1 : start_x;
    start_y = (start_y < 0) ? 0 : (start_y >= HEIGHT) ? HEIGHT-1 : start_y;
    int end_x = start_x + size;
    int end_y = start_y + size;
    end_x = (end_x > WIDTH) ? WIDTH : end_x;
    end_y = (end_y > HEIGHT) ? HEIGHT : end_y;
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            image[y * WIDTH + x] = value;
        }
    }
}

int main() {
    printf("Testing create_square_rvv function...\n");
    // Allocate and initialize image buffer
    uint8_t *image = (uint8_t *)calloc(WIDTH * HEIGHT, sizeof(uint8_t));
    if (!image) {
        printf("Memory allocation failed\n");
        return 1;
    }
    printf("Creating 10x10 white square at center (256,256)...\n");
    // Test with a simple case first
    startv=clock();
    for(int i=0;i<steps;i++) create_square_rvv(image, 256, 256, 10, 255);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    // Verify the result by checking if any pixels were set
    int count = 0;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        if (image[i] == 255) count++;
    }
    printf("Pixels set to white: %d (expected: 100)\n", count);
    // Save the result
    save_pgm("square_rvv.pgm", image, WIDTH, HEIGHT);
    // Test edge cases
    printf("Testing edge case: square at corner...\n");
    memset(image, 0, WIDTH * HEIGHT); // Clear image
                                      //
    //create_square_rvv(image, 5, 5, 20, 255);
    start=clock();
    for(int i=0;i<steps;i++) create_square_scalar(image, 5, 5, 20, 255);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    save_pgm("square_corner.pgm", image, WIDTH, HEIGHT);
    free(image);
    printf("Test completed successfully!\n");
    return 0;
}

