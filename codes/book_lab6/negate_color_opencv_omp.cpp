// negate_color_opencv_omp.cpp
#include <cstdio>
#include <cstdint>
#include <omp.h>
#include <opencv2/opencv.hpp>

constexpr int WIDTH = 512;
constexpr int HEIGHT = 512;
constexpr int CHANNELS = 3; // BGR

static void negate_serial(const cv::Mat& in, cv::Mat& out) {
    out.create(in.size(), in.type());
    if (in.isContinuous() && out.isContinuous()) {
        const size_t n = in.total() * in.elemSize();
        const unsigned char* src = in.ptr<unsigned char>(0);
        unsigned char* dst = out.ptr<unsigned char>(0);
        for (size_t i = 0; i < n; ++i)
            dst[i] = static_cast<unsigned char>(255u - src[i]);
    } else {
        for (int y = 0; y < in.rows; ++y) {
            const unsigned char* src = in.ptr<unsigned char>(y);
            unsigned char* dst = out.ptr<unsigned char>(y);
            for (int i = 0; i < in.cols * in.channels(); ++i)
                dst[i] = static_cast<unsigned char>(255u - src[i]);
        }
    }
}

static void negate_omp8(const cv::Mat& in, cv::Mat& out) {
    out.create(in.size(), in.type());
    if (in.isContinuous() && out.isContinuous()) {
        const ptrdiff_t n = static_cast<ptrdiff_t>(in.total() * in.elemSize());
        const unsigned char* src = in.ptr<unsigned char>(0);
        unsigned char* dst = out.ptr<unsigned char>(0);
        #pragma omp parallel for simd schedule(static) num_threads(8)
        for (ptrdiff_t i = 0; i < n; ++i)
            dst[i] = static_cast<unsigned char>(255u - src[i]);
    } else {
        #pragma omp parallel for schedule(static) num_threads(8)
        for (int y = 0; y < in.rows; ++y) {
            const unsigned char* src = in.ptr<unsigned char>(y);
            unsigned char* dst = out.ptr<unsigned char>(y);
            for (int i = 0; i < in.cols * in.channels(); ++i)
                dst[i] = static_cast<unsigned char>(255u - src[i]);
        }
    }
}
int main(int argc, char** argv) {
    const char* inpath = (argc > 1) ? argv[1] : "input.png";
    cv::Mat img = cv::imread(inpath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::fprintf(stderr, "Error: cannot load image '%s'\n", inpath);
        return 1;
    }
    if (img.cols != WIDTH || img.rows != HEIGHT)
        cv::resize(img, img, cv::Size(WIDTH, HEIGHT), 0, 0, cv::INTER_AREA);
    cv::Mat outSerial, outOMP;
    // Warm-up
    negate_serial(img, outSerial);
    negate_omp8(img, outOMP);
    // Time serial
    double t0 = omp_get_wtime();
    negate_serial(img, outSerial);
    double t1 = omp_get_wtime();

    // Time OpenMP (8 threads)
    double t2 = omp_get_wtime();
    negate_omp8(img, outOMP);
    double t3 = omp_get_wtime();
    // === FIXED equality check for 3-channel images ===
    bool same = (cv::norm(outSerial, outOMP, cv::NORM_INF) == 0.0);
        // (Alternative):
    // cv::Mat diff; cv::absdiff(outSerial, outOMP, diff);
    // same = (cv::countNonZero(diff.reshape(1)) == 0);
    std::printf("Results equal : %s\n", same ? "YES" : "NO");
    std::printf("Serial  time  : %.6f s\n", t1 - t0);
    std::printf("OpenMP (8)    : %.6f s\n", t3 - t2);
    if ((t3 - t2) > 0.0)
        std::printf("Speedup       : %.2fx\n", (t1 - t0) / (t3 - t2));
    if (!cv::imwrite("negated_color.png", outOMP))
        std::fprintf(stderr, "Warning: failed to save 'negated_color.png'\n");
    else
        std::printf("Wrote negated_color.png (%dx%d)\n", WIDTH, HEIGHT);
    return same ? 0 : 2;
}

