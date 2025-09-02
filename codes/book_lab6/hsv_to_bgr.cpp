// hsv_to_bgr.cpp
#include <cstdio>
#include <opencv2/opencv.hpp>

static inline void ensure_size(cv::Mat& m, int w, int h) {
    if (m.cols != w || m.rows != h)
        cv::resize(m, m, cv::Size(w, h), 0, 0, cv::INTER_AREA);
}

int main(int argc, char** argv) {
    const char* inpath  = (argc > 1) ? argv[1] : "input_hsv.png";
    const char* outpath = (argc > 2) ? argv[2] : "output_bgr.png";

    // Load HSV as-is (keep bit depth)
    cv::Mat hsv = cv::imread(inpath, cv::IMREAD_UNCHANGED);
    if (hsv.empty()) {
        std::fprintf(stderr, "Error: cannot load '%s'\n", inpath);
        return 1;
    }
    if (hsv.channels() != 3) {
        std::fprintf(stderr, "Error: expected 3-channel HSV image, got %d channels\n", hsv.channels());
        return 2;
    }

    // Ensure 512x512 (as requested)
    ensure_size(hsv, 512, 512);

    cv::Mat bgr8;

    if (hsv.type() == CV_8UC3) {
        // OpenCV 8-bit HSV expects H in [0..179], S,V in [0..255].
        // If H looks like 0..255, rescale to 0..179.
        std::vector<cv::Mat> ch;
        cv::split(hsv, ch); // H,S,V
        double minH, maxH;
        cv::minMaxLoc(ch[0], &minH, &maxH);
        if (maxH > 179.0) {
            ch[0].convertTo(ch[0], CV_8U, 179.0 / 255.0);
            cv::merge(ch, hsv);
            std::puts("Info: Rescaled H 0..255 → 0..179 for 8-bit HSV.");
        }

        // Convert HSV(8U) → BGR(8U)  (OpenCV’s native channel order)
        cv::cvtColor(hsv, bgr8, cv::COLOR_HSV2BGR);
    } else if (hsv.type() == CV_32FC3 || hsv.type() == CV_64FC3) {
        // Float HSV: H in [0..360], S,V in [0..1]
        cv::Mat hsv32;
        if (hsv.type() == CV_64FC3) hsv.convertTo(hsv32, CV_32FC3);
        else                        hsv32 = hsv;

        // HSV(float) → BGR(float) → BGR(8U)
        cv::Mat bgr32;
        cv::cvtColor(hsv32, bgr32, cv::COLOR_HSV2BGR);
        bgr32.convertTo(bgr8, CV_8UC3, 255.0);

    } else {
        std::fprintf(stderr, "Error: unsupported HSV type (%d). Use CV_8UC3 or CV_32FC3.\n", hsv.type());
        return 3;
    }

    // Save BGR image. Note: cv::imwrite expects BGR data and will write a standard file;
    // viewers interpret the PNG correctly—no extra RGB swap needed here.
    if (!cv::imwrite(outpath, bgr8)) {
        std::fprintf(stderr, "Error: failed to write '%s'\n", outpath);
        return 4;
    }
    std::printf("Wrote %s (%dx%d, BGR in memory / standard PNG on disk)\n",
                outpath, bgr8.cols, bgr8.rows);

    // (Optional) If you specifically need an RGB-ordered file buffer before saving:
    // cv::Mat rgb8; cv::cvtColor(bgr8, rgb8, cv::COLOR_BGR2RGB); cv::imwrite("output_rgb.png", rgb8);

    return 0;
}

