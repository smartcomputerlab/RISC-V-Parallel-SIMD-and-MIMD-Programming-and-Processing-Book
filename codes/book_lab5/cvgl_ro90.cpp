// cv_opengl_drawpixels_rotate_512.cpp
// Keep one CPU buffer (RGB, 8UC3, 512x512). Every 2s, rotate it 90° CW
// and display via glDrawPixels.

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>

#if defined(__APPLE__)
  #include <OpenGL/gl.h>
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif

static constexpr int W = 512;
static constexpr int H = 512;

static std::vector<uint8_t> gBuf;   // current image buffer (W*H*3, RGB)
static std::vector<uint8_t> gTmp;   // scratch for rotation
static int gWinW = W, gWinH = H;
static unsigned long gTick = 0;

clock_t startv, endv;
clock_t start, end;
double elapsed_secv;
double elapsed_sec;
int steps=10;

extern "C"  void rotate90_rvv(const uint8_t* src, uint8_t* dst, size_t N);

// Inject: copy an RGB Mat (512x512, 8UC3) into gBuf
static void injectImageToBuffer(const cv::Mat& rgb) {
    if (rgb.cols != W || rgb.rows != H || rgb.type() != CV_8UC3) return;
    const size_t bytes = (size_t)W * H * 3;
    if (!rgb.isContinuous()) {
        cv::Mat tmp = rgb.clone();
        memcpy(gBuf.data(), tmp.data, bytes);
    } else {
        memcpy(gBuf.data(), rgb.data, bytes);
    }
}

// Rotate 90° CW: (r,c) -> (c, N-1-r). Operates on RGB interleaved.
static void rotate90_cw_rgb(const uint8_t* src, uint8_t* dst) {
    const int N = W; // 512
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            const int si = (r * N + c) * 3;
            const int di = (c * N + (N - 1 - r)) * 3;
            dst[di + 0] = src[si + 0];
            dst[di + 1] = src[si + 1];
            dst[di + 2] = src[si + 2];
        }
	    }
}

static void displayCB() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Draw with origin upper-left: flip Y via negative PixelZoom
    glRasterPos2f(-1.f,  1.f);
    glPixelZoom((float)gWinW / W, -(float)gWinH / H);
    glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, gBuf.data());
    glPixelZoom(1.f, 1.f);

    glutSwapBuffers();
}

static void reshapeCB(int w, int h) {
    gWinW = (w > 1) ? w : 1;
    gWinH = (h > 1) ? h : 1;
    glViewport(0, 0, gWinW, gWinH);
}

static void keyCB(unsigned char key, int, int) {
    if (key == 27 || key == 'q') { // ESC / q
#ifdef FREEGLUT
        //glutLeaveMainLoop();
#endif
        exit(0);
    }
    if (key == 'r') { // rotate immediately
        rotate90_cw_rgb(gBuf.data(), gTmp.data());
        gBuf.swap(gTmp);
        glutPostRedisplay();
    }
    if (key == 'o') { // reload original file on demand (no path stored here)
        // noop placeholder; reload logic can be added if you store the path
    }
}

// Every 2000 ms: rotate current buffer 90° CW, update title, redraw
static void timerCB(int) {
    ++gTick;

    start=clock();
    for(int i=0;i<steps;i++) rotate90_cw_rgb(gBuf.data(), gTmp.data());
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);

    startv=clock();
    for(int i=0;i<steps;i++) rotate90_rvv(gBuf.data(), gTmp.data(), 512);
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);
    gBuf.swap(gTmp);

    char title[160];
    snprintf(title, sizeof(title), "Pixels 512x512 — rotate 90 CW");
    glutSetWindowTitle(title);

    glutPostRedisplay();
    glutTimerFunc(200, timerCB, 0); // re-arm for 2s
}


static bool loadAndPrep(const char* path) {
    cv::Mat src = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (src.empty()) {
        printf("Error: cannot open %s\n", path);
        return false;
    }
    if (src.cols != W || src.rows != H) {
        printf("Note: input is %dx%d, resizing to %dx%d.\n", src.cols, src.rows, W, H);
	        cv::resize(src, src, cv::Size(W, H), 0, 0, cv::INTER_AREA);
    }
    cv::Mat rgb;
    if (src.channels() == 1)       cv::cvtColor(src, rgb, cv::COLOR_GRAY2RGB);
    else if (src.channels() == 3)  cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);
    else if (src.channels() == 4)  cv::cvtColor(src, rgb, cv::COLOR_BGRA2RGB);
    else {
        printf("Unsupported channel count: %d\n", src.channels());
        return false;
    }
    injectImageToBuffer(rgb);
    return true;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <image_512x512.(png|jpg|...)>\n", argv[0]);
        return 1;
    }

    gBuf.resize((size_t)W * H * 3);
    gTmp.resize((size_t)W * H * 3);
    if (!loadAndPrep(argv[1])) return 1;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("Pixels 512x512 — rotate 90cw degrees per tick");
    glClearColor(0.f, 0.f, 0.f, 1.f);

    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyCB);
    glutTimerFunc(2000, timerCB, 0); // start periodic rotation

    glutMainLoop();
    return 0;
}

