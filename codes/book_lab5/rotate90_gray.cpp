#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define N 512  // image is 512x512
extern "C" void rotate90_gray_rvv(const uint8_t *src, uint8_t *dst);

// ------------------- "namespace" ns_: plain C-style prefix -------------------
// 90° clockwise: (r, c) -> (c, N-1-r)
//
static void rotate90_cw_u8_c(const uint8_t *src, uint8_t *dst) {
    for (int r = 0; r < N; ++r) {
        const uint8_t *srow = src + r * N;
        for (int c = 0; c < N; ++c) {
            dst[c * N + (N - 1 - r)] = srow[c];
        }
    }
}
int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <grayscale_512x512.png>  <rot_image_ass.png> <rot_image_c>\n", argv[0]);
        return 1;
    }
    const char *in_path = argv[1];
    const char *out_path_ass = argv[2];
    const char *out_path_c = argv[3];
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    // Load as 8-bit grayscale with OpenCV
    cv::Mat img = cv::imread(in_path, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        printf("Error: could not open image: %s\n", in_path);
        return 1;
    }
    if (img.cols != N || img.rows != N) {
        printf("Error: expected %dx%d, got %dx%d\n", N, N, img.cols, img.rows);
        return 1;
    }
    if (img.type() != CV_8UC1) {
        printf("Error: expected 8-bit single-channel image.\n");
        return 1;
    }
    if (!img.isContinuous()) {
        img = img.clone(); // ensure tight rows for pointer math
    }

// Rotate using the C-style function
    uint8_t *dst_ass = (uint8_t*)malloc((size_t)N * N);
    uint8_t *dst_c = (uint8_t*)malloc((size_t)N * N);
    if (!dst_ass || !dst_c) {
        printf("Error: out of memory\n");
        return 1;
    }
    startv=clock();
    rotate90_gray_rvv(img.data, dst_ass);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    start=clock();
    rotate90_cw_u8_c(img.data, dst_c);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Wrap output buffer and save with OpenCV
    cv::Mat rot_ass(N, N, CV_8UC1, dst_ass);
    cv::Mat rot_c(N, N, CV_8UC1, dst_c);
    if (!cv::imwrite(out_path_ass, rot_ass)) {
        printf("Error: failed to write %s\n", out_path_ass);
        free(dst_ass);
        return 1;
    }
    if (!cv::imwrite(out_path_c, rot_c)) {
        printf("Error: failed to write %s\n", out_path_c);
        free(dst_c);
        return 1;
    }
    printf("Wrote: %s\n", out_path_ass);
    printf("Wrote: %s\n", out_path_c);
    cv::imshow("Rotatds assembler",rot_ass);
    cv::imshow("Rotated C",rot_c);
    cv::waitKey(0);
    free(dst_ass);
    free(dst_c);
    return 0;
}

