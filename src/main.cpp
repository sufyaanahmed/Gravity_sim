#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GLFWwindow* StartGLFW();
void DrawSphere(float radius, int slices, int stacks);
void DrawFloor(float y, float width, float depth);
void DrawGrid(float size, float step);
void initLighting();

float camRadius = 600.0f;
float camTheta = M_PI / 2.0f;  // horizontal angle
float camPhi = M_PI / 4.0f;    // vertical angle
float camX, camY, camZ;

bool rotating = false;
double lastX = 0, lastY = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        rotating = (action == GLFW_PRESS);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rotating) {
        float dx = (float)(xpos - lastX);
        float dy = (float)(ypos - lastY);

        camTheta -= dx * 0.005f;
        camPhi   += dy * 0.005f;

        if (camPhi < -M_PI / 2 + 0.1f) camPhi = -M_PI / 2 + 0.1f;
        if (camPhi >  M_PI / 2 - 0.1f) camPhi =  M_PI / 2 - 0.1f;
    }

    lastX = xpos;
    lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= yoffset * 10.0f;
    if (camRadius < 100.0f) camRadius = 100.0f;
    if (camRadius > 2000.0f) camRadius = 2000.0f;
}


class Sphere {
public:
    float radius;
    float position[3];
    float velocity[3];
    float color[3];

    Sphere(float r, float x, float y, float z, float rCol, float gCol, float bCol)
        : radius(r)
    {
        position[0] = x; position[1] = y; position[2] = z;
        velocity[0] = 0.0f; velocity[1] = 0.0f; velocity[2] = 0.0f;
        color[0] = rCol; color[1] = gCol; color[2] = bCol;
    }

    void Update(float deltaTime, float gravity, float worldWidth, float worldDepth) {
        // Apply gravity
        velocity[1] += gravity * deltaTime;
        for (int i = 0; i < 3; ++i)
            position[i] += velocity[i] * deltaTime;

        // Bounce floor
        if (position[1] - radius < 0) {
            position[1] = radius;
            velocity[1] *= -0.7f;
            if (fabsf(velocity[1]) < 10.0f) velocity[1] = 0;
        }

        // Walls X
        if (position[0] + radius > worldWidth / 2) {
            position[0] = worldWidth / 2 - radius;
            velocity[0] *= -0.7f;
        } else if (position[0] - radius < -worldWidth / 2) {
            position[0] = -worldWidth / 2 + radius;
            velocity[0] *= -0.7f;
        }

        // Walls Z
        if (position[2] + radius > worldDepth / 2) {
            position[2] = worldDepth / 2 - radius;
            velocity[2] *= -0.7f;
        } else if (position[2] - radius < -worldDepth / 2) {
            position[2] = -worldDepth / 2 + radius;
            velocity[2] *= -0.7f;
        }

        // Damping
        velocity[0] *= 0.99f;
        velocity[2] *= 0.99f;
    }

    void Draw(int slices, int stacks) {
        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);
        glColor3f(color[0], color[1], color[2]);
        DrawSphere(radius, slices, stacks);
        glPopMatrix();
    }
};


int main() {
    GLFWwindow* window = StartGLFW();
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    initLighting();

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);


    float radius = 30.0f;
    int slices = 20, stacks = 20;

    float worldHeight = 800.0f, worldWidth = 1000.0f, worldDepth = 400.0f;
    float prevTime = glfwGetTime();
    float gravity = -980.0f;

    float position[3] = { 0.0f, 200.0f, 0.0f };
    float velocity[3] = { 0.0f, 0.0f, 0.0f };

    Sphere ball1(30.0f, -200.0f, 200.0f, 0.0f, 1.0f, 0.0f, 0.0f); // Red, left
    Sphere ball2(30.0f,  200.0f, 200.0f, 0.0f, 0.0f, 0.0f, 1.0f); // Blue, right


    while (!glfwWindowShouldClose(window)) {
        float currTime = glfwGetTime();
        float deltaTime = currTime - prevTime;   //calculate the time diff between each frame
        prevTime = currTime;
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera & Projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float fov = 45.0f, aspect = 800.0f / 600.0f;
        float near = 0.1f, far = 1000.0f;
        float top = tan(fov * 0.5f * M_PI / 180.0f) * near;
        float right = top * aspect;
        glFrustum(-right, right, -top, top, near, far);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        camX = camRadius * cosf(camPhi) * sinf(camTheta);
        camY = camRadius * sinf(camPhi);
        camZ = camRadius * cosf(camPhi) * cosf(camTheta);
        gluLookAt(camX, camY, camZ, 0, 0, 0, 0, 1, 0);
        
        // Physics
        velocity[1] += gravity * deltaTime;
        for (int i = 0; i < 3; i++)
        position[i] += velocity[i] * deltaTime;
        
        // Bounce floor
        if (position[1] - radius < 0) {
            position[1] = radius;
            velocity[1] *= -0.7f;
            if (fabs(velocity[1]) < 10.0f) velocity[1] = 0;
        }

        // Walls
        if (position[0] + radius > worldWidth / 2) {
            position[0] = worldWidth / 2 - radius;
            velocity[0] *= -0.7f;
        } else if (position[0] - radius < -worldWidth / 2) {
            position[0] = -worldWidth / 2 + radius;
            velocity[0] *= -0.7f;
        }

        if (position[2] + radius > worldDepth / 2) {
            position[2] = worldDepth / 2 - radius;
            velocity[2] *= -0.7f;
        } else if (position[2] - radius < -worldDepth / 2) {
            position[2] = -worldDepth / 2 + radius;
            velocity[2] *= -0.7f;
        }

        velocity[0] *= 0.99f;
        velocity[2] *= 0.99f;
        
        // Draw Sphere
        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);
        glColor3f(1.0f, 1.0f, 1.0f);
        //DrawSphere(radius, slices, stacks);
        ball1.Update(deltaTime, gravity, worldWidth, worldDepth);
        ball2.Update(deltaTime, gravity, worldWidth, worldDepth);

        ball1.Draw(slices, stacks);
        ball2.Draw(slices, stacks);

        glPopMatrix();
        
        //DrawFloor(0.0f, 800.0f, 400.0f);
        DrawGrid(1000.0f,50.0f);
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    }

    glfwTerminate();
    return 0;
}

GLFWwindow* StartGLFW() {
    if (!glfwInit()) return nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    return glfwCreateWindow(800, 600, "3D Gravity Sim (No GLU/GLUT)", NULL, NULL);
}

void DrawFloor(float y, float width, float depth) {
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.5f, 0.2f); // greenish floor

    glNormal3f(0, 1, 0);
    glVertex3f(-width/2, y, -depth/2);
    glVertex3f( width/2, y, -depth/2);
    glVertex3f( width/2, y,  depth/2);
    glVertex3f(-width/2, y,  depth/2);

    glEnd();
    glPopMatrix();
}

void DrawSphere(float radius, int slices, int stacks) {   //stacks => latitudes , slices => longitudes
    for (int i = 0; i < stacks; ++i) {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float z0 = sin(lat0), zr0 = cos(lat0);
        
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
        float z1 = sin(lat1), zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2 * M_PI * (float)j / slices;
            float x = cos(lng), y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0 * radius, y * zr0 * radius, z0 * radius);

            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1 * radius, y * zr1 * radius, z1 * radius);            
        }
        glEnd();
    }
}

void DrawGrid(float size, float step){
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.3f);
    
    glBegin(GL_LINES);
    for(float i= -size; i <= size ; i += step){
        //vertical lines (x axis)
        glVertex3f(i, 0, -step);
        glVertex3f(i, 0, step);

        //horizontal lines (z axis)
        glVertex3f(-size,0,i);
        glVertex3f(size,0,i);
    }
    glEnd();
    glPopMatrix();

}

void initLighting() {
    glEnable(GL_LIGHTING);        // Enable lighting system
    glEnable(GL_LIGHT0);          // Enable default light
    glEnable(GL_COLOR_MATERIAL);  // Allow glColor to affect materials
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); // Material settings

    // Set up light position
    GLfloat light_pos[] = { 0.0f, 500.0f, 500.0f, 1.0f }; // Positional light
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    // Set light properties
    GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
}

