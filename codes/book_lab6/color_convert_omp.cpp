// color_convert_omp.cpp
#include <cstdio>
#include <cmath>
#include <omp.h>
#include <opencv2/opencv.hpp>

constexpr int WIDTH  = 512;
constexpr int HEIGHT = 512;

/* Correct BGR(8-bit) -> HSV(float, H in [0,360), S,V in [0,1]) */
static inline void bgr_to_hsv(uint8_t B8, uint8_t G8, uint8_t R8,
                              float &H, float &S, float &V)
{
    const float b = B8 * (1.0f/255.0f);
    const float g = G8 * (1.0f/255.0f);
    const float r = R8 * (1.0f/255.0f);
    const float cmax = std::max(r, std::max(g, b));
    const float cmin = std::min(r, std::min(g, b));
    const float d = cmax - cmin;
    // Hue
    if (d == 0.0f) {
        H = 0.0f;
    } else if (cmax == r) {
        float h = (g - b) / d;        // in (-∞,∞)
        if (h < 0.0f) h += 6.0f;      // wrap into [0,6)
        H = 60.0f * h;
    } else if (cmax == g) {
        H = 60.0f * ((b - r) / d + 2.0f);
    } else { // cmax == b
        H = 60.0f * ((r - g) / d + 4.0f);
    }
    // Saturation
    S = (cmax <= 0.0f) ? 0.0f : (d / cmax);
    // Value
    V = cmax;
}

/* Serial conversion: CV_8UC3 (BGR) -> CV_32FC3 (HSV) */
static void bgr2hsv_serial(const cv::Mat &bgr, cv::Mat &hsv)
{
    hsv.create(bgr.rows, bgr.cols, CV_32FC3);
    for (int y = 0; y < bgr.rows; ++y) {
        const uint8_t *src = bgr.ptr<uint8_t>(y);
        float *dst = hsv.ptr<float>(y);
        for (int x = 0; x < bgr.cols; ++x) {
            float H,S,V;
            bgr_to_hsv(src[3*x+0], src[3*x+1], src[3*x+2], H,S,V);
            dst[3*x+0] = H;  // degrees 0..360
            dst[3*x+1] = S;  // 0..1
            dst[3*x+2] = V;  // 0..1
        }
    }
}


/* OpenMP (8 threads) row-parallel conversion */
static void bgr2hsv_omp8(const cv::Mat &bgr, cv::Mat &hsv)
{
    hsv.create(bgr.rows, bgr.cols, CV_32FC3);
    #pragma omp parallel for schedule(static) num_threads(8)
    for (int y = 0; y < bgr.rows; ++y) {
        const uint8_t *src = bgr.ptr<uint8_t>(y);
        float *dst = hsv.ptr<float>(y);
        #pragma omp simd
        for (int x = 0; x < bgr.cols; ++x) {
            float H,S,V;
            bgr_to_hsv(src[3*x+0], src[3*x+1], src[3*x+2], H,S,V);
            dst[3*x+0] = H;
            dst[3*x+1] = S;
            dst[3*x+2] = V;
        }
    }
    }

/* Compute max abs diff per channel between two CV_32FC3 images */
static void max_abs_diff3(const cv::Mat &A, const cv::Mat &B, double d[3])
{
    std::vector<cv::Mat> ca, cb;
    cv::split(A, ca);
    cv::split(B, cb);
    for (int k = 0; k < 3; ++k)
        d[k] = cv::norm(ca[k], cb[k], cv::NORM_INF);
}
int main(int argc, char **argv)
{
    const char *path = (argc > 1) ? argv[1] : "input.png";

    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR); // BGR
    if (img.empty()) {
        std::fprintf(stderr, "Error: cannot load '%s'\n", path);
        return 1;
    }
    if (img.cols != WIDTH || img.rows != HEIGHT)
        cv::resize(img, img, cv::Size(WIDTH, HEIGHT), 0, 0, cv::INTER_AREA);

    cv::Mat hsv_serial, hsv_omp, hsv_cv;

    // Warm-up
    bgr2hsv_serial(img, hsv_serial);
    bgr2hsv_omp8(img, hsv_omp);

    // Time serial
    double t0 = omp_get_wtime();
    bgr2hsv_serial(img, hsv_serial);
    double t1 = omp_get_wtime();

    // Time OpenMP
    double t2 = omp_get_wtime();
    bgr2hsv_omp8(img, hsv_omp);
    double t3 = omp_get_wtime();

    // Reference using OpenCV (float HSV: H 0..360, S,V 0..1)
    cv::Mat img_f32;
    img.convertTo(img_f32, CV_32F, 1.0/255.0);
    cv::cvtColor(img_f32, hsv_cv, cv::COLOR_BGR2HSV); // float output

    // Compare: our serial vs OpenCV
    double d_ser[3], d_omp[3];
    max_abs_diff3(hsv_serial, hsv_cv, d_ser);
    max_abs_diff3(hsv_omp,    hsv_cv, d_omp);

    std::printf("Max |Δ| vs OpenCV (H,S,V):\n");
    std::printf("  Serial : H=%.6g  S=%.6g  V=%.6g\n", d_ser[0], d_ser[1], d_ser[2]);
    std::printf("  OMP(8) : H=%.6g  S=%.6g  V=%.6g\n", d_omp[0], d_omp[1], d_omp[2]);

    std::printf("Serial  time : %.6f s\n", t1 - t0);
    std::printf("OpenMP (8)   : %.6f s\n", t3 - t2);
    if ((t3 - t2) > 0.0)
        std::printf("Speedup      : %.2fx\n", (t1 - t0) / (t3 - t2));

    // (Optional) Save HSV as 8-bit visualizations for inspection
    cv::Mat hsv8_show;
    cv::Mat H = hsv_omp.clone(), S = hsv_omp.clone(), V = hsv_omp.clone();
    std::vector<cv::Mat> ch; cv::split(hsv_omp, ch);
    ch[0].convertTo(ch[0], CV_8U, 255.0/360.0); // H degrees -> 0..255
    ch[1].convertTo(ch[1], CV_8U, 255.0);       // S 0..1 -> 0..255
    ch[2].convertTo(ch[2], CV_8U, 255.0);       // V 0..1 -> 0..255
    cv::merge(ch, hsv8_show);
    cv::imwrite("hsv_omp_vis.png", hsv8_show);

    return 0;
}

