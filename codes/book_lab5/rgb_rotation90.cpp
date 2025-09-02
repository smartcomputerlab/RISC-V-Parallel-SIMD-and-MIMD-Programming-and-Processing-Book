#include <opencv2/opencv.hpp>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;

extern "C" void rotate_rgb_90ccw_rvv(const uint8_t* src, int srcW, int srcH, int srcStep, uint8_t* dst, int dstW, int dstH, int dstStep);

void rotate_rgb_90cw(const uint8_t* src, int srcW, int srcH, int srcStep, uint8_t* dst, int dstW, int dstH, int dstStep)
{
    if (!src || !dst) return;
    if (dstW != srcH || dstH != srcW) {
        printf("Error: dst size must be (srcH x srcW) for 90-degree rotation.\n");
        return;
    }
    for (int y = 0; y < srcH; ++y) {
        const uint8_t* srow = src + (size_t)y * srcStep;
        for (int x = 0; x < srcW; ++x) {
            const uint8_t* spix = srow + 3 * x;
            // Mapping: (x,y) in source -> (y, dstH-1-x) in dest
            int dx = y;
            int dy = dstH - 1 - x;
            uint8_t* drow = dst + (size_t)dy * dstStep;
            uint8_t* dpix = drow + 3 * dx;
            dpix[0] = spix[0];
            dpix[1] = spix[1];
            dpix[2] = spix[2];
        }
    }
}

void rotate_rgb_90ccw(const uint8_t* src, int srcW, int srcH, int srcStep,
                      uint8_t* dst, int dstW, int dstH, int dstStep)
{
    if (!src || !dst) return;
    if (dstW != srcH || dstH != srcW) {
        printf("Error: dst size must be (srcH x srcW) for 90-degree rotation.\n");
        return;
    }
    for (int y = 0; y < srcH; ++y) {
        const uint8_t* srow = src + (size_t)y * srcStep;
        for (int x = 0; x < srcW; ++x) {
            const uint8_t* spix = srow + 3 * x;
            // Mapping: (x,y) in source -> (dstW-1-y, x) in dest
            int dx = dstW - 1 - y;
            int dy = x;
            uint8_t* drow = dst + (size_t)dy * dstStep;
            uint8_t* dpix = drow + 3 * dx;
            dpix[0] = spix[0];
            dpix[1] = spix[1];
            dpix[2] = spix[2];
        }
    }
}

// ---------------------------------------------------------
// Demo program using OpenCV
// ---------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Usage: %s input.png [output.png]\n", argv[0]);
        return 1;
    }
    const char* inPath  = argv[1];
    const char* outPath = (argc > 2) ? argv[2] : "rotated90.png";
    // Read color image
    cv::Mat src = cv::imread(inPath, cv::IMREAD_COLOR);
    if (src.empty()) {
        printf("Error: cannot read %s\n", inPath);
        return 2;
    }
    printf("Loaded %s: %dx%d\n", inPath, src.cols, src.rows);
    // Destination size = (srcH x srcW)
    cv::Mat dst(src.cols, src.rows, CV_8UC3);
    // Rotate 90° clockwise
    start=clock();
    rotate_rgb_90cw(src.data, src.cols, src.rows, (int)src.step, dst.data, dst.cols, dst.rows, (int)dst.step);
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    startv=clock();
    rotate_rgb_90ccw_rvv(src.data, src.cols, src.rows, (int)src.step, dst.data, dst.cols, dst.rows, (int)dst.step);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    // Save & show
    cv::imwrite(outPath, dst);
    printf("Wrote rotated image: %s\n", outPath);
    cv::imshow("Original", src);
    cv::imshow("Rotated 90 CW", dst);
    printf("Press any key in the image window to exit...\n");
    cv::waitKey(0);
    return 0;
}

