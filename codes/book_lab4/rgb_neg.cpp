// rgb_neg.cpp
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
using namespace cv;

extern "C" void  negate_buffer_rvv(const uint8_t* inbuf, uint8_t* outbuf, int width, int height, int channels);
// Create negated buffer (out = 255 - in)
void  negate_buffer(const uint8_t* inbuf, uint8_t* outbuf, int width, int height, int channels) {
    size_t size = (size_t)width * height * channels;
    for (size_t i = 0; i < size; i++) {
        outbuf[i] = 255 - inbuf[i];
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return -1;
    }
    // Load input image as color
    Mat img = imread(argv[1], IMREAD_COLOR);
    if (img.empty()) {
        printf("Could not open or find the image: %s\n", argv[1]);
        return -1;
    }
    int width = img.cols;
    int height = img.rows;
    int channels = img.channels();
    size_t bufsize = (size_t)width * height * channels;
    // Copy Mat data into raw buffer
    uint8_t* buffer_in = (uint8_t*)malloc(bufsize);
    uint8_t* buffer_out = (uint8_t*)malloc(bufsize);
    memcpy(buffer_in, img.data, bufsize);
    start=clock();
    // Call the C function
    negate_buffer(buffer_in, buffer_out, width, height, channels);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    startv=clock();
    // Call the assembly function
    negate_buffer_rvv(buffer_in, buffer_out, width, height, channels);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Wrap buffers into Mat for display and saving
    Mat orig(height, width, (channels == 3 ? CV_8UC3 : CV_8UC1), buffer_in);
    Mat neg(height, width, (channels == 3 ? CV_8UC3 : CV_8UC1), buffer_out);
    // Save negated image
    if (imwrite("rgb_neg.png", neg)) {
        printf("Negated image saved as: rgb_neg.png\n");
    } else {
        printf("Failed to save rgb_neg.png\n");
    }
    // Show both images
    imshow("Original Image", orig);
    imshow("Negated Image", neg);
    waitKey(0);
    // Clean up
    free(buffer_in);
    free(buffer_out);
    return 0;
}

