// dda_algorithm.cpp
// Draws line segments with the Digital Differential Analyzer (DDA) algorithm
// using legacy (fixed-function) OpenGL + GLFW.
//
// Controls:
//   Left click  : first click sets the start point, second click sets the end
//                 point and draws a DDA line between them
//   Right click : clear all lines
//   ESC         : quit

#include <glfw3.h>    // also includes the OpenGL header (gl.h)
#include <algorithm>       // std::max
#include <cmath>           // std::abs, std::lround
#include <cstdio>          // printf
#include <cstdlib>         // std::abs(int)
#include <vector>

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------
struct Segment { int x1, y1, x2, y2; };
std::vector<Segment> userLines;      // lines created by mouse clicks
bool havePending = false;            // is the first click of a pair stored?
int  pendingX = 0, pendingY = 0;

// ---------------------------------------------------------------------------
// DDA algorithm
// Plots every pixel of the line from (x1,y1) to (x2,y2).
// Must be called between glBegin(GL_POINTS) and glEnd().
// ---------------------------------------------------------------------------
void ddaPlot(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;                          // total change in x
    int dy = y2 - y1;                          // total change in y

    // Number of steps = the larger of |dx| and |dy|.
    int steps = std::max(std::abs(dx), std::abs(dy));

    if (steps == 0) {                          // start == end: a single point
        glVertex2i(x1, y1);
        return;
    }

    // Per-step increments. One of them is always +1 or -1;
    // the other has magnitude <= 1.
    float xInc = static_cast<float>(dx) / steps;
    float yInc = static_cast<float>(dy) / steps;

    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);

    for (int i = 0; i <= steps; ++i) {
        // Round to the nearest pixel and plot it.
        glVertex2i(static_cast<int>(std::lround(x)),
            static_cast<int>(std::lround(y)));
        x += xInc;
        y += yInc;
    }
}

// Prints the DDA calculation table to the console so it can be checked by hand.
void printDDATrace(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1, dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps == 0) { printf("DDA trace: start and end are the same point\n\n"); return; }
    float xInc = static_cast<float>(dx) / steps;
    float yInc = static_cast<float>(dy) / steps;
    float x = static_cast<float>(x1), y = static_cast<float>(y1);

    printf("DDA trace: (%d,%d) -> (%d,%d)\n", x1, y1, x2, y2);
    printf("dx=%d dy=%d steps=%d xInc=%.4f yInc=%.4f\n", dx, dy, steps, xInc, yInc);
    printf("%3s %9s %9s   %s\n", "i", "x", "y", "plotted");
    for (int i = 0; i <= steps; ++i) {
        printf("%3d %9.4f %9.4f   (%ld,%ld)\n", i, x, y, std::lround(x), std::lround(y));
        x += xInc;
        y += yInc;
    }
    printf("\n");
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
void render(GLFWwindow* window)
{
    // Window size (screen coordinates) is what the mouse uses;
    // framebuffer size (real pixels) is what the viewport uses.
    // They differ on high-DPI displays.
    int winW, winH, fbW, fbH;
    glfwGetWindowSize(window, &winW, &winH);
    glfwGetFramebufferSize(window, &fbW, &fbH);
    if (winW == 0 || winH == 0) return;        // minimised

    glViewport(0, 0, fbW, fbH);

    // 2D coordinate system: origin bottom-left, 1 unit = 1 window pixel.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, winW, 0.0, winH, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(2.0f);                         // make single pixels visible

    // --- Lines drawn by the user (yellow) ---
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POINTS);
    for (const Segment& s : userLines)
        ddaPlot(s.x1, s.y1, s.x2, s.y2);
    glEnd();

    // Mark a pending first click in red.
    if (havePending) {
        glColor3f(1.0f, 0.2f, 0.2f);
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glVertex2i(pendingX, pendingY);
        glEnd();
    }
}

// ---------------------------------------------------------------------------
// GLFW callbacks
// ---------------------------------------------------------------------------
void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (action != GLFW_PRESS) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);    // position in window coordinates
        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);

        int x = static_cast<int>(mx);
        int fy = winH - static_cast<int>(my);  // window y grows downward; OpenGL's grows upward

        if (!havePending) {
            pendingX = x;
            pendingY = fy;
            havePending = true;
        }
        else {
            userLines.push_back({ pendingX, pendingY, x, fy });
            printDDATrace(pendingX, pendingY, x, fy);
            havePending = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        userLines.clear();
        havePending = false;
    }
}

void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void errorCallback(int code, const char* description)
{
    fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialise GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "DDA Line Drawing", nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);                       // sync to monitor refresh rate

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    printf("Left click twice to draw a DDA line (start point, then end point).\n");
    printf("Right click clears the window. ESC quits.\n\n");

    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}