// rgb2gray_omp.cpp
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <omp.h>
#include <opencv2/opencv.hpp>

constexpr int WIDTH  = 512;
constexpr int HEIGHT = 512;

/* Serial RGB (BGR in OpenCV) -> Grayscale, CV_8UC3 -> CV_8UC1 */
static void rgb2gray_serial(const cv::Mat& color, cv::Mat& gray) {
    gray.create(color.rows, color.cols, CV_8UC1);
    for (int y = 0; y < color.rows; ++y) {
        const uint8_t* src = color.ptr<uint8_t>(y);
        uint8_t* dst = gray.ptr<uint8_t>(y);
        for (int x = 0; x < color.cols; ++x) {
            const int b = src[3*x + 0];
            const int g = src[3*x + 1];
            const int r = src[3*x + 2];
            // Y ≈ (0.114*B + 0.587*G + 0.299*R)
            const int y8 = (29*b + 150*g + 77*r + 128) >> 8;
            dst[x] = static_cast<uint8_t>(y8);
        }
    }
}

/* OpenMP (8 threads) version */
static void rgb2gray_omp8(const cv::Mat& color, cv::Mat& gray) {
    gray.create(color.rows, color.cols, CV_8UC1);
    #pragma omp parallel for schedule(static) num_threads(8)
    for (int y = 0; y < color.rows; ++y) {
        const uint8_t* src = color.ptr<uint8_t>(y);
        uint8_t* dst = gray.ptr<uint8_t>(y);
        for (int x = 0; x < color.cols; ++x) {
            const int b = src[3*x + 0];
            const int g = src[3*x + 1];
            const int r = src[3*x + 2];
            const int y8 = (29*b + 150*g + 77*r + 128) >> 8;
            dst[x] = static_cast<uint8_t>(y8);
        }
    }
}

int main(int argc, char** argv) {
    const char* inpath = (argc > 1) ? argv[1] : "input.png";

    // Load BGR color image
    cv::Mat img = cv::imread(inpath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::fprintf(stderr, "Error: cannot load image '%s'\n", inpath);
        return 1;
    }
    if (img.cols != WIDTH || img.rows != HEIGHT)
        cv::resize(img, img, cv::Size(WIDTH, HEIGHT), 0, 0, cv::INTER_AREA);

    cv::Mat gray_serial, gray_omp;

    // Warm-up
    rgb2gray_serial(img, gray_serial);
    rgb2gray_omp8(img, gray_omp);

    // Time serial
    double t0 = omp_get_wtime();
    rgb2gray_serial(img, gray_serial);
    double t1 = omp_get_wtime();

    // Time OpenMP (8 threads)
    double t2 = omp_get_wtime();
    rgb2gray_omp8(img, gray_omp);
    double t3 = omp_get_wtime();

    // Compare outputs (both are single-channel)
    bool same = (cv::norm(gray_serial, gray_omp, cv::NORM_INF) == 0.0);

    std::printf("Results equal : %s\n", same ? "YES" : "NO");
    std::printf("Serial  time  : %.6f s\n", t1 - t0);
    std::printf("OpenMP (8)    : %.6f s\n", t3 - t2);
    if ((t3 - t2) > 0.0)
        std::printf("Speedup       : %.2fx\n", (t1 - t0) / (t3 - t2));

    // Save parallel result
    if (!cv::imwrite("gray_omp.png", gray_omp))
        std::fprintf(stderr, "Warning: failed to save 'gray_omp.png'\n");
    else
        std::printf("Wrote gray_omp.png (%dx%d)\n", WIDTH, HEIGHT);

    return same ? 0 : 2;
}

