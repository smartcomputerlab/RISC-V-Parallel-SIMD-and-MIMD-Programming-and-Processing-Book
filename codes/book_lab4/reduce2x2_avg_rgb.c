#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
clock_t startv, endv;
clock_t start, end;
double elapsed_secv;
double elapsed_sec;
extern "C" void reduce2x2_avg_rgb_rvv(const uint8_t* src, int srcStep, uint8_t* dst, int dstStep, int srcW, int srcH);

static void reduce2x2_avg_rgb(const uint8_t* src, int srcStep, uint8_t* dst, int dstStep, int srcW, int srcH)
{
    const int dstW = srcW >> 1;   // assume even dimensions
    const int dstH = srcH >> 1;
    for (int y = 0; y < dstH; ++y) {
        const uint8_t* row0 = src + (2 * y) * srcStep;      // top row of block
        const uint8_t* row1 = row0 + srcStep;               // bottom row of block
        uint8_t* out = dst + y * dstStep;
        for (int x = 0; x < dstW; ++x) {
            const uint8_t* p00 = row0 + (2 * x) * 3;        // top-left
            const uint8_t* p01 = p00 + 3;                   // top-right
            const uint8_t* p10 = row1 + (2 * x) * 3;        // bottom-left
            const uint8_t* p11 = p10 + 3;                   // bottom-right
            // average each channel (R, G, B)
            for (int c = 0; c < 3; ++c) {
                unsigned sum = p00[c] + p01[c] + p10[c] + p11[c];
                out[3 * x + c] = (uint8_t)((sum + 2) >> 2);  // divide by 4 with rounding
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <color_image_512x512>\n", argv[0]);
        return 1;
    }
    const char* in_path = argv[1];
    // Load color (BGR)
    cv::Mat img = cv::imread(in_path, cv::IMREAD_COLOR);
    if (img.empty()) {
        printf("Error: cannot open %s\n", in_path);
        return 1;
    }
    if (img.cols != 512 || img.rows != 512) {
        printf("Error: expected 512x512 input, got %dx%d (no cv::resize allowed in this task).\n",
               img.cols, img.rows);
        return 1;
    }
    if (!img.isContinuous()) img = img.clone(); // ensure tight rows
    // Prepare output buffer (256x256, 3 channels)
    const int dstW = 256, dstH = 256;
    cv::Mat small(dstH, dstW, CV_8UC3);
    // Manual reduction on raw byte buffers
    start=clock();
    reduce2x2_avg_rgb(img.data, (int)img.step, small.data, (int)small.step, img.cols, img.rows);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    startv=clock();
    reduce2x2_avg_rgb_rvv(img.data, (int)img.step, small.data, (int)small.step, img.cols, img.rows);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Display result
    cv::imshow("Reduced 256x256", small);
    cv::imwrite("reduced.png", small);
    printf("Showing reduced 256x256 image. Press any key to exit.\n");
    cv::waitKey(0);
    return 0;
}

