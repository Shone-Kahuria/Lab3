#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846f

const int WIDTH = 800, HEIGHT = 600;

// Globe Parameters
const float GLOBE_RADIUS = 0.6f;
const int LATITUDE_LINES = 12;   // Number of horizontal slices
const int LONGITUDE_LINES = 18;  // Number of vertical meridians
const int SEGMENTS_PER_LINE = 60; // Smoothness of each line

// Animation Angle
float rotationY = 0.0f;

// Draw a horizontal line of latitude (circle at fixed height y)
void drawLatitudeLine(float latitudeAngle)
{
    float y = GLOBE_RADIUS * sinf(latitudeAngle);
    float radiusSlice = GLOBE_RADIUS * cosf(latitudeAngle);

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < SEGMENTS_PER_LINE; i++)
    {
        float theta = 2.0f * PI * i / SEGMENTS_PER_LINE;
        float x = radiusSlice * cosf(theta);
        float z = radiusSlice * sinf(theta);

        glVertex3f(x, y, z);
    }
    glEnd();
}

// Draw a vertical line of longitude (ellipse/great circle rotated around Y axis)
void drawLongitudeLine(float longitudeAngle)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < SEGMENTS_PER_LINE; i++)
    {
        float phi = 2.0f * PI * i / SEGMENTS_PER_LINE;

        // Unrotated circle in the YZ plane
        float x0 = 0.0f;
        float y0 = GLOBE_RADIUS * sinf(phi);
        float z0 = GLOBE_RADIUS * cosf(phi);

        // Rotate point around Y-axis by longitudeAngle
        float x = x0 * cosf(longitudeAngle) + z0 * sinf(longitudeAngle);
        float y = y0;
        float z = -x0 * sinf(longitudeAngle) + z0 * cosf(longitudeAngle);

        glVertex3f(x, y, z);
    }
    glEnd();
}

// Render complete wireframe globe
void renderGlobe()
{
    glPushMatrix();

    // Tilt the globe slightly (23.5 degrees) and rotate continuously
    glRotatef(23.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

    // Set wireframe color (Cyan/Light Blue)
    glColor3f(0.2f, 0.8f, 1.0f);

    // 1. Draw Lines of Latitude
    for (int i = 1; i < LATITUDE_LINES; i++)
    {
        float phi = -PI / 2.0f + (PI * i / LATITUDE_LINES);
        drawLatitudeLine(phi);
    }

    // 2. Draw Lines of Longitude
    for (int i = 0; i < LONGITUDE_LINES; i++)
    {
        float theta = PI * i / LONGITUDE_LINES;
        drawLongitudeLine(theta);
    }

    glPopMatrix();
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        printf("GLFW initialization failed!\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Wireframe Globe Model (Ellipse Equation)", NULL, NULL);
    if (!window)
    {
        printf("GLFW window creation failed!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW initialization failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Enable Depth Testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);

    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);

    // Main Render Loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Clear screen and depth buffer
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update rotation for continuous spinning
        rotationY += 0.3f;
        if (rotationY > 360.0f) rotationY -= 360.0f;

        // Render the wireframe globe
        renderGlobe();

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}