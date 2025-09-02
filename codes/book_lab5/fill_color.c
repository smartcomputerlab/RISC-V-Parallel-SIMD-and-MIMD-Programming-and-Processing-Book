
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define WIDTH  640
#define HEIGHT 480

extern "C" void fill_color_rvv(uint8_t *buffer, uint8_t r, uint8_t g, uint8_t b);

static GLuint tex = 0;
static uint8_t *frame = NULL;
static int color_index = 0; // 0 = red, 1 = green, 2 = blue
static int color_red = 0;
static int color_green = 0;
static int color_blue = 0;
			    //

static void fill_color(uint8_t *buffer , uint8_t r, uint8_t g, uint8_t b)
{
    for (int y = 0; y < HEIGHT; ++y) {
        uint8_t *row = buffer + (size_t)y * WIDTH * 3;
        for (int x = 0; x < WIDTH; ++x) {
            row[3*x + 0] = r;
            row[3*x + 1] = g;
            row[3*x + 2] = b;
        }
    }
}


static void init_texture(void)
{
    frame = (unsigned char*)malloc((size_t)WIDTH * HEIGHT * 3);
    if (!frame) { fprintf(stderr, "OOM\n"); exit(1); }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

static void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)WIDTH, 0.0, (GLdouble)HEIGHT);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}


static void display(void)
{
    clock_t startv, endv;
    clock_t start, end;
    double elapsed_secv;
    double elapsed_sec;
    int step=10;
    startv=clock();
    for(int i=0;i<step;i++) fill_color_rvv(frame,color_red, color_green, color_blue);   
    endv=clock();
    elapsed_secv = (double)(endv - startv) / CLOCKS_PER_SEC;
    printf("Vector execution time in sec: %.6f\n",elapsed_secv);
    start=clock();
    for(int i=0;i<step;i++) fill_color(frame,color_red, color_green, color_blue);   
    end=clock();
    elapsed_sec = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Scalar execution time in sec: %.6f\n",elapsed_sec);
    printf("Speed-up: %.3f\n",elapsed_sec/elapsed_secv);

    // Upload frame
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT,
                    GL_RGB, GL_UNSIGNED_BYTE, frame);

    // Draw
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(0.f,      0.f);
        glTexCoord2f(1.f, 0.f); glVertex2f(WIDTH,    0.f);
        glTexCoord2f(1.f, 1.f); glVertex2f(WIDTH,    HEIGHT);
        glTexCoord2f(0.f, 1.f); glVertex2f(0.f,      HEIGHT);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}

static void timer(int value)
{

    (void)value;
    // Move to next color
    //color_index = (color_index + 1) % 3;
    color_red = rand()%256;   // (color_red + 1) % 256;
    color_green = rand()%256;   //(color_green + 1) % 256;
    color_blue = rand()%256;   //(color_blue + 1) % 256;
    glutPostRedisplay();
    glutTimerFunc(100, timer, 0); // change color every 1 second
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Simple RGB Animation (640x480)");

    glClearColor(0.f, 0.f, 0.f, 1.f);

    reshape(WIDTH, HEIGHT);
    init_texture();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    free(frame);
    return 0;
}

