
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

const int WIDTH = 800;
const int HEIGHT = 600;

// Store each pixel as an (x, y) pair in OpenGL coordinates.
std::vector<float> vertices;

// Bresenham's Line Drawing Algorithm.
// This version handles positive, negative, steep and shallow slopes.
void bresenhamLine(int x1, int y1, int x2, int y2)
{
    int dx = std::abs(x2 - x1);
    int dy = -std::abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int error = dx + dy;

    while (true)
    {
        // Convert pixel coordinates to OpenGL Normalized Device Coordinates.
        float xNDC = 2.0f * x1 / (WIDTH - 1) - 1.0f;
        float yNDC = 2.0f * y1 / (HEIGHT - 1) - 1.0f;

        vertices.push_back(xNDC);
        vertices.push_back(yNDC);

        // Stop after plotting the final endpoint.
        if (x1 == x2 && y1 == y2)
            break;

        int error2 = 2 * error;

        if (error2 >= dy)
        {
            error += dy;
            x1 += sx;
        }

        if (error2 <= dx)
        {
            error += dx;
            y1 += sy;
        }
    }
}

// Compile a shader and report errors.
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n"
            << infoLog << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createShaderProgram()
{
    const char* vertexShaderSource = R"(
        #version 330 core

        layout (location = 0) in vec2 aPos;

        void main()
        {
            gl_Position = vec4(aPos, 0.0, 1.0);
            gl_PointSize = 3.0;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core

        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )";

    GLuint vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    GLuint fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        if (vertexShader != 0)
            glDeleteShader(vertexShader);

        if (fragmentShader != 0)
            glDeleteShader(fragmentShader);

        return 0;
    }

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n"
            << infoLog << '\n';
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (!success)
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Bresenham's Line Drawing Algorithm",
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    // Test different line slopes.
    // Coordinates use a bottom-left origin.
    bresenhamLine(60, 80, 330, 150);     // Positive, shallow
    bresenhamLine(80, 180, 150, 480);    // Positive, steep
    bresenhamLine(450, 150, 740, 80);    // Negative, shallow
    bresenhamLine(650, 480, 720, 180);   // Negative, steep
    bresenhamLine(350, 300, 450, 300);   // Horizontal
    bresenhamLine(400, 350, 400, 500);   // Vertical

    GLuint shaderProgram = createShaderProgram();

    if (shaderProgram == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Rendering loop.
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glDrawArrays(
            GL_POINTS,
            0,
            static_cast<GLsizei>(vertices.size() / 2)
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}