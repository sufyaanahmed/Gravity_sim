#include <iostream>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <vector>

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
    }

    return shader;
}

GLuint CreateShaderProgram(const char* vertSrc, const char* fragSrc) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragSrc);
    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Shader link error:\n" << log << "\n";
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
void DrawSphere(float radius, int slices, int stacks);

class Sphere {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float color[3];
    float mass;

    Sphere(float r, float x, float y, float z, float rCol, float gCol, float bCol, float m)
        : radius(r), mass(m)
    {
        position[0] = x; position[1] = y; position[2] = z;
        velocity[0] = 0.0f; velocity[1] = 0.0f; velocity[2] = 0.0f;
        color[0] = rCol; color[1] = gCol; color[2] = bCol;
    }

    /*
    void Update(float deltaTime, float gravity, float worldWidth, float worldDepth) {
        // Apply gravity
        //velocity[1] += gravity * deltaTime;
        float force = mass * gravity;
        float acceleration = mass / force;
        velocity[1] += acceleration * deltaTime;
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
    */

    void ApplyForce(const glm::vec3& force, float dt) {
        glm::vec3 acceleration = force / mass;
        velocity += acceleration * dt;
        velocity.y = 0.0f; //Preventing the movement of the y-axis
    }

    void Update(float dt) {
        position += velocity * dt;
        position.y = 25.0f;
    }

    void Draw(int slices, int stacks) {
        glPushMatrix();
        glTranslatef(position[0], position[1], position[2]);
        glColor3f(color[0], color[1], color[2]);
        DrawSphere(radius, slices, stacks);
        glPopMatrix();
    }
};

GLFWwindow* StartGLFW();
void DrawFloor(float y, float width, float depth);
void DrawGrid(float size, float step);
void initLighting();
void HyperBoloid_Funnel_WithMass(std::vector<Sphere>& planets, float size, float step);

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

const char* vertexShaderSource = R"(
#version 120
varying vec3 vNormal;
void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vNormal = gl_NormalMatrix * gl_Normal;
    gl_FrontColor = gl_Color;
}
)";

const char* fragmentShaderSource = R"(
#version 120
varying vec3 vNormal;
void main() {
    float lighting = max(dot(normalize(vNormal), vec3(0, 0, 1)), 0.3);
    gl_FragColor = vec4(gl_Color.rgb * lighting, 1.0);
}
)";





int main() {
    GLFWwindow* window = StartGLFW();
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    GLuint planetShader = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

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

    //float position[3] = { 0.0f, 200.0f, 0.0f };
    //float velocity[3] = { 0.0f, 0.0f, 0.0f };

    std::vector<Sphere> planets;

    // Sun
    planets.emplace_back(60.0f, 0.0f, 25.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.989e6f);

    // Mercury
    Sphere mercury(5.0f, 100.0f, 25.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.33f);
    mercury.velocity.z = 130.0f;
    planets.push_back(mercury);

    // Venus
    Sphere venus(10.0f, 150.0f, 25.0f, 0.0f, 0.9f, 0.7f, 0.2f, 4.87f);
    venus.velocity.z = 108.0f;
    planets.push_back(venus);

    // Earth
    Sphere earth(12.0f, 200.0f, 25.0f, 0.0f, 0.2f, 0.2f, 1.0f, 5.97f);
    earth.velocity.z = 98.0f;
    planets.push_back(earth);

    // Mars
    Sphere mars(10.0f, 250.0f, 25.0f, 0.0f, 1.0f, 0.3f, 0.1f, 0.64f);
    mars.velocity.z = 85.0f;
    planets.push_back(mars);

    // Jupiter
    Sphere jupiter(25.0f, 350.0f, 25.0f, 0.0f, 1.0f, 0.9f, 0.6f, 1898.0f);
    jupiter.velocity.z = 50.0f;
    planets.push_back(jupiter);
    /*
    // dummy planet with the mass of sun
    Sphere dummy(25.0f, 350.0f, 25.0f, 0.0f, 1.0f, 0.9f, 0.6f, 1.989e6f);
    dummy.velocity.z = 50.0f;
    planets.push_back(dummy);
    */

    // Saturn
    Sphere saturn(22.0f, 450.0f, 25.0f, 0.0f, 1.0f, 0.8f, 0.4f, 568.0f);
    saturn.velocity.z = 40.0f;
    planets.push_back(saturn);

    // Uranus
    Sphere uranus(18.0f, 550.0f, 25.0f, 0.0f, 0.6f, 0.8f, 1.0f, 86.8f);
    uranus.velocity.z = 30.0f;
    planets.push_back(uranus);

    // Neptune
    Sphere neptune(17.0f, 650.0f, 25.0f, 0.0f, 0.4f, 0.4f, 1.0f, 102.0f);
    neptune.velocity.z = 25.0f;
    planets.push_back(neptune);


    //ball1.velocity = glm::vec3(0.0f, 0.0f, 20.0f);  // optional initial nudge
    



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
        /*
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
        */

        //velocity[0] *= 0.99f;
        //velocity[2] *= 0.99f;
        
        // Draw Spherefor this 
        glPushMatrix();
        //glTranslatef(position[0], position[1], position[2]);
        glColor3f(1.0f, 1.0f, 1.0f);
        for (size_t i = 0; i < planets.size(); ++i) {
            glm::vec3 totalForce(0.0f);

            for (size_t j = 0; j < planets.size(); ++j) {
                if (i == j) continue;

                glm::vec3 dir = planets[j].position - planets[i].position;
                float dist2 = glm::dot(dir, dir) + 1.0f;
                float dist = sqrt(dist2);
                glm::vec3 dirNorm = dir / dist;

                float G = 6.67430e-1f; // Scaled gravitational constant
                float forceMag = G * planets[i].mass * planets[j].mass / dist2;
                totalForce += dirNorm * forceMag;
            }

            planets[i].ApplyForce(totalForce, deltaTime);
        }
        glUseProgram(planetShader);
        for (auto& planet : planets) {
            planet.Update(deltaTime);
            planet.Draw(slices, stacks);
        }
        glUseProgram(0);


        glPopMatrix();
        
        //DrawFloor(0.0f, 800.0f, 400.0f);
        //DrawGrid(1000.0f,50.0f);
        HyperBoloid_Funnel_WithMass(planets, 500.0f, 15.0f);
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

void HyperBoloid_Funnel_WithMass(std::vector<Sphere>& planets, float size, float step) {
    glPushMatrix();
    glColor3f(0.9f, 0.9f, 0.9f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (float x = -size; x < size; x += step) {
        for (float z = -size; z < size; z += step) {
            float y00 = 0.0f, y10 = 0.0f, y01 = 0.0f, y11 = 0.0f;

            for (const auto& p : planets) {
                float dx0 = x - p.position.x, dz0 = z - p.position.z;
                float dx1 = x + step - p.position.x, dz1 = z + step - p.position.z;

                float dist00 = dx0*dx0 + dz0*dz0 + 1.0f;
                float dist10 = dx1*dx1 + dz0*dz0 + 1.0f;
                float dist01 = dx0*dx0 + dz1*dz1 + 1.0f;
                float dist11 = dx1*dx1 + dz1*dz1 + 1.0f;

                y00 -= glm::min(p.mass / dist00 * 0.03f, 50.0f);
                y10 -= glm::min(p.mass / dist10 * 0.03f, 50.0f);
                y01 -= glm::min(p.mass / dist01 * 0.03f, 50.0f);
                y11 -= glm::min(p.mass / dist11 * 0.03f, 50.0f);

            }

            float x0 = x, z0 = z;
            float x1 = x + step, z1 = z + step;

            glBegin(GL_TRIANGLES);
            glVertex3f(x0, y00, z0);
            glVertex3f(x1, y10, z0);
            glVertex3f(x1, y11, z1);

            glVertex3f(x0, y00, z0);
            glVertex3f(x1, y11, z1);
            glVertex3f(x0, y01, z1);
            glEnd();
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPopMatrix();
}
