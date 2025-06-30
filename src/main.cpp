#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

GLFWwindow* StartGLFW();
void DrawCircle(float centerX, float centerY, float radius, int res);

int main() {
    GLFWwindow* window = StartGLFW();
    if (!window) {
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);

    // Set up 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    float radius = 50.0f;
    int res = 100;

    std::vector<float> position = {400.0f, 500.0f};
    std::vector<float> velocity = {0.0f, 0.0f};

    float gravity = -980.0f; // pixels per second squared
    float prevTime = glfwGetTime();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - prevTime;
        prevTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);

        // Physics calculations
        velocity[1] += gravity * deltaTime;
        position[0] += velocity[0] * deltaTime;
        position[1] += velocity[1] * deltaTime;

        // Apply damping (air resistance)
        velocity[0] *= 0.99f;

        // Collision with floor
        if (position[1] - radius < 0) {
            position[1] = radius;
            velocity[1] *= -0.7f;
            if (std::abs(velocity[1]) < 5.0f) {
                velocity[1] = 0.0f;
            }
        }

        // Collision with ceiling
        if (position[1] + radius > screenHeight) {
            position[1] = screenHeight - radius;
            velocity[1] *= -0.7f;
        }

        // Collision with walls
        if (position[0] - radius < 0) {
            position[0] = radius;
            velocity[0] *= -0.7f;
        }
        if (position[0] + radius > screenWidth) {
            position[0] = screenWidth - radius;
            velocity[0] *= -0.7f;
        }

        DrawCircle(position[0], position[1], radius, res);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLFW() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "gravity_sim", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    return window;
}

void DrawCircle(float centerX, float centerY, float radius, int res) {
    glColor3f(1.0f, 1.0f, 1.0f);  // White color
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(centerX, centerY);
    for (int i = 0; i <= res; i++) {
        float angle = 2.0f * 3.1415926f * (static_cast<float>(i) / res);
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}
