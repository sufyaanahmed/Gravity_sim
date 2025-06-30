#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

GLFWwindow* StartGLFW();
void DrawCircle(float centerX, float centerY, float radius, int res);

int main(){
    GLFWwindow* window = StartGLFW();
    if (!window) {
        return -1;
    }

    // Set up OpenGL context
    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set viewport
    glViewport(0, 0, 800, 600);
    
    // Set up 2D orthographic projection for pixel coordinates
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);  // Left, Right, Bottom, Top, Near, Far
    // Now Y=0 is bottom, Y=600 is top (like a math graph)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    
    float screenWidth = 800.0f;
    float screenHeight = 600.0f;
    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;
    float radius = 50.0f;
    int res = 100;

    std::vector<float> position={400.0f, 500.0f};  // Start near top
    std::vector<float> velocity = {0.0f, 0.0f};

    // Set background color to dark blue
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    

    while(!glfwWindowShouldClose(window)){

        glClear(GL_COLOR_BUFFER_BIT);

        DrawCircle(position[0], position[1], radius, res);
        //position[1] -= 1.0f;  // Decrease Y to move DOWN (gravity effect)
        position[0] += velocity[0];
        position[1] += velocity[1];
        velocity[1] += -9.81 / 20.0f;

        if(position[1] < 0 || position[1] >screenHeight){
            velocity[1] *= -0.95;
        }
        if(position[0] < 0 || position[0] >screenWidth){
            velocity[0] *= -0.95;
        }

        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLFW(){

    if(!glfwInit()){
        std::cerr<<"failed to initialize glfw, panic!!"<<std::endl;
        return nullptr;
    }
    
    // Set OpenGL version hints for compatibility with immediate mode rendering
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
    for(int i=0; i<=res; i++){
        float angle = 2.0f * 3.1415926f * (static_cast<float>(i) / res);
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

