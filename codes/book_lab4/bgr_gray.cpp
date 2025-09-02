#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
using namespace cv;
#define W 512
#define H 512
clock_t startv, endv;
clock_t start, end;
double elapsed_secv;
double elapsed_sec;
extern "C" void bgr_gray_rvv(const uint8_t* bgr, uint8_t* gray, size_t pixels);
// Integer luma: gray ≈ 0.299R + 0.587G + 0.114B
// OpenCV loads color as BGR, so: (29*B + 150*G + 77*R) >> 8
static inline uint8_t bgr_to_gray(uint8_t b, uint8_t g, uint8_t r) {
    return (uint8_t)(((29u * b) + (150u * g) + (77u * r) + 128u) >> 8);
}
// Copy a Mat (CV_8UC3) into a tight BGR buffer (width*height*3), row by row
bool copy_mat_to_bgr_buffer(const Mat& img, uint8_t* dst_tight) {
    if (img.type() != CV_8UC3 || img.cols != W || img.rows != H) return false;
    const int src_step = (int)img.step;        // bytes per row (may be > W*3)
    const int row_bytes = W * 3;               // tight row size
    const uint8_t* src = img.ptr<uint8_t>(0);
    for (int r = 0; r < H; ++r) {
        memcpy(dst_tight + r * row_bytes, src + r * src_step, row_bytes);
    }
    return true;
}
// Convert tight BGR buffer -> tight GRAY buffer
void bgr_gray(const uint8_t* bgr, uint8_t* gray) {
    const size_t pixels = (size_t)W * H;
    for (size_t i = 0; i < pixels; ++i) {
        const uint8_t B = bgr[3*i + 0];
        const uint8_t G = bgr[3*i + 1];
        const uint8_t R = bgr[3*i + 2];
        gray[i] = bgr_to_gray(B, G, R);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return -1;
    }
    //Load as color (BGR, 3 channels)
    Mat src = imread(argv[1], IMREAD_COLOR);
    if (src.empty()) {
        printf("Could not open or find the image: %s\n", argv[1]);
        return -1;
    }
    Mat img;
    resize(src, img, Size(W, H), 0, 0, INTER_LINEAR);  // enforce size
    // Allocate tight raw buffers
    const size_t bgr_size  = (size_t)W * H * 3;
    const size_t gray_size = (size_t)W * H;
    uint8_t* bgr_buf  = (uint8_t*)malloc(bgr_size);
    uint8_t* gray_buf = (uint8_t*)malloc(gray_size);
    if (!bgr_buf || !gray_buf) {
        printf("Out of memory\n");
        free(bgr_buf); free(gray_buf);
        return -1;
    }
    // Copy Mat (which might have padding) into tight BGR buffer
    if (!copy_mat_to_bgr_buffer(img, bgr_buf)) {
        printf("Unexpected image format/size\n");
        free(bgr_buf); free(gray_buf);
        return -1;
    }
    // Convert to grayscale in a tight buffer
    start=clock();
    bgr_gray(bgr_buf, gray_buf);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    // Convert to grayscale in a tight buffer
    startv=clock();
    bgr_gray_rvv(bgr_buf, gray_buf, W*H);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Wrap the tight buffers into Mats for display/save (no extra copies)
    Mat orig(H, W, CV_8UC3, bgr_buf);
    Mat gray(H, W, CV_8UC1, gray_buf);
    // Show both images (both are exactly 512x512)
    imshow("Original 512x512 (BGR)", orig);
    imshow("Grayscale 512x512", gray);
    // Save the grayscale result (optional)
    imwrite("grayscale_512x512.png", gray);
    waitKey(0);
    // 10) Clean up
    free(bgr_buf);
    free(gray_buf);
    return 0;
}

