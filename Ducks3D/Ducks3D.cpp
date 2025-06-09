#define _USE_MATH_DEFINES

#include <iostream>
#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/ResourceManager.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Submarine3D", monitor, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glViewport(0, 0, mode->width, mode->height);

    float planeVertices[] = {
        // positions          // tex coords
        -50.0f, 0.0f, -50.0f,  0.0f, 0.0f,  // 0
         50.0f, 0.0f, -50.0f,  10.0f, 0.0f, // 1
         50.0f, 0.0f,  50.0f,  10.0f, 10.0f,// 2
        -50.0f, 0.0f,  50.0f,  0.0f, 10.0f  // 3
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::vector<float> lakeVertices;
    const int segments = 128;
    const float radius = 25.0f;
    const float y = 0.1f; // slight elevation to prevent z-fighting
    const float tileFactor = 10.0f; // how many times texture repeats across the lake

    // Center vertex
    lakeVertices.push_back(0.0f); // x
    lakeVertices.push_back(y);    // y
    lakeVertices.push_back(0.0f); // z
    lakeVertices.push_back(tileFactor * 0.5f); // u
    lakeVertices.push_back(tileFactor * 0.5f); // v

    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        lakeVertices.push_back(x);
        lakeVertices.push_back(y);
        lakeVertices.push_back(z);
        lakeVertices.push_back(tileFactor * (0.5f + 0.5f * cos(angle))); // u
        lakeVertices.push_back(tileFactor * (0.5f + 0.5f * sin(angle))); // v
    }

    GLuint lakeVAO, lakeVBO;
    glGenVertexArrays(1, &lakeVAO);
    glGenBuffers(1, &lakeVBO);

    glBindVertexArray(lakeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lakeVBO);
    glBufferData(GL_ARRAY_BUFFER, lakeVertices.size() * sizeof(float), lakeVertices.data(), GL_STATIC_DRAW);

    // Position attribute (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates attribute (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ResourceManager::loadShader("resources/shaders/basic.vert", "resources/shaders/basic.frag", nullptr, "shader");
    ResourceManager::loadTexture("resources/textures/grass.jpg", false, "grass");
    ResourceManager::loadTexture("resources/textures/water.jpg", false, "water");

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(
            glm::vec3(150.0f, 150.0f, 150.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f),    
            glm::vec3(0.0f, 1.0f, 0.0f)     
        );
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f, 1000.0f
        );

        ResourceManager::getShader("shader").Use().SetMatrix4("model", model);
        ResourceManager::getShader("shader").SetMatrix4("view", view);
        ResourceManager::getShader("shader").SetMatrix4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        ResourceManager::getTexture("grass").Bind();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glActiveTexture(GL_TEXTURE0);
        ResourceManager::getTexture("water").Bind();

        glBindVertexArray(lakeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}