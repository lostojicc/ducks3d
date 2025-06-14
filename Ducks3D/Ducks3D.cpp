#define _USE_MATH_DEFINES

#include <iostream>
#include <vector>
#include <cmath>
#include <random>   
#include <chrono>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utility/ResourceManager.h"
#include "utility/model-loading/Model.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const float CAMERA_SPEED = 1.5;

const int TARGET_FPS = 60;
const std::chrono::duration<double, std::milli> FRAME_DURATION(1000.0 / TARGET_FPS);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float cameraZoom = 100.0f;
float cameraAngle = 0.0f;
float cameraElevation = glm::radians(45.0f); 

float rotationSpeed = 1.5f;
float rotationAngle = 0.0f;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.3f, 0.7f);
    float duckSizeMultipliers[3] = {
        dist(gen),
        dist(gen),
        dist(gen)
    };

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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glViewport(0, 0, mode->width, mode->height);

    float planeVertices[] = {
        -200.0f, 0.0f, -200.0f,    0.0f, 0.0f,   
         200.0f, 0.0f, -200.0f,   20.0f, 0.0f,   
         200.0f, 0.0f,  200.0f,   20.0f, 20.0f,  
        -200.0f, 0.0f,  200.0f,    0.0f, 20.0f   
    };

    unsigned int indices[] = {
        0, 2, 1,   
        0, 3, 2
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    std::vector<float> lakeVertices;
    const int segments = 50;
    const float radius = 50.0f;
    const float y = 0.1f; 
    const float tileFactor = 5.0f; 

    lakeVertices.push_back(0.0f); 
    lakeVertices.push_back(y);   
    lakeVertices.push_back(0.0f); 
    lakeVertices.push_back(tileFactor * 0.5f); 
    lakeVertices.push_back(tileFactor * 0.5f); 

    for (int i = segments; i >= 0; --i) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        lakeVertices.push_back(x);
        lakeVertices.push_back(y);
        lakeVertices.push_back(z);
        lakeVertices.push_back(tileFactor * (0.5f + 0.5f * cos(angle))); 
        lakeVertices.push_back(tileFactor * (0.5f + 0.5f * sin(angle))); 
    }

    GLuint lakeVAO, lakeVBO;
    glGenVertexArrays(1, &lakeVAO);
    glGenBuffers(1, &lakeVBO);

    glBindVertexArray(lakeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lakeVBO);
    glBufferData(GL_ARRAY_BUFFER, lakeVertices.size() * sizeof(float), lakeVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    float signatureQuad[] = {
        -0.95f, -0.95f,    0.0f, 0.0f,  
        -0.50f, -0.95f,    1.0f, 0.0f,  
        -0.50f, -0.75f,    1.0f, 1.0f,  
        -0.95f, -0.75f,    0.0f, 1.0f   
    };

    unsigned int signatureIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    GLuint sigVAO, sigVBO, sigEBO;
    glGenVertexArrays(1, &sigVAO);
    glGenBuffers(1, &sigVBO);
    glGenBuffers(1, &sigEBO);

    glBindVertexArray(sigVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sigVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureQuad), signatureQuad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sigEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(signatureIndices), signatureIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ResourceManager::loadShader("resources/shaders/basic.vert", "resources/shaders/basic.frag", nullptr, "shader");
    ResourceManager::loadShader("resources/shaders/signature.vert", "resources/shaders/signature.frag", nullptr, "signatureShader");

    ResourceManager::loadTexture("resources/textures/grass.jpg", false, "grass");
    ResourceManager::loadTexture("resources/textures/water.jpg", false, "water");
    ResourceManager::loadTexture("resources/textures/duck.png", true, "duck");
    ResourceManager::loadTexture("resources/textures/signature.png", true, "signature");

    Model duck("resources/models/duck.obj");

    

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);        
    glCullFace(GL_BACK);          
 
    glm::vec3 cameraPos;

    while (!glfwWindowShouldClose(window)) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        
        ResourceManager::getShader("signatureShader").Use();
        glActiveTexture(GL_TEXTURE0);
        ResourceManager::getTexture("signature").Bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(sigVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        

        glm::mat4 model = glm::mat4(1.0f);
        
        float x = sin(cameraElevation) * sin(cameraAngle) * cameraZoom;
        float y = cos(cameraElevation) * cameraZoom;
        float z = sin(cameraElevation) * cos(cameraAngle) * cameraZoom;

        cameraPos = glm::vec3(x, y, z);

        glm::mat4 view = glm::lookAt(
            cameraPos,             
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

        glActiveTexture(GL_TEXTURE0);
        ResourceManager::getTexture("duck").Bind();
        rotationAngle += rotationSpeed * deltaTime;

        model = glm::mat4(1.0f);
        model = glm::rotate(model, -rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(30.0f, 0.0f, 0.0f));
        ResourceManager::getShader("shader").SetMatrix4("model", model);
        duck.Draw();

        ResourceManager::getShader("shader").SetVector3f("color", glm::vec3(1.0f, 1.0f, 0.0f));

        for (int i = 0; i < 3; ++i) {
            float offset = glm::radians(30.0f + i * 15.0f);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, -rotationAngle + offset, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate(model, glm::vec3(30.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(duckSizeMultipliers[i]));
            ResourceManager::getShader("shader").SetMatrix4("model", model);
            duck.Draw();
        }
        ResourceManager::getShader("shader").SetVector3f("color", glm::vec3(1.0f, 1.0f, 1.0f));

        glfwSwapBuffers(window);
        glfwPollEvents();

        auto frameEnd = std::chrono::high_resolution_clock::now();
        auto elapsed = frameEnd - frameStart;
        if (elapsed < FRAME_DURATION) 
            std::this_thread::sleep_for(FRAME_DURATION - elapsed);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = CAMERA_SPEED * deltaTime;
    
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraAngle -= cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraAngle += cameraSpeed; 

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        rotationSpeed += 0.5f * deltaTime;   
        if (rotationSpeed > 3.0f)             
            rotationSpeed = 3.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        rotationSpeed -= 0.5f * deltaTime;   
        if (rotationSpeed < 0.0f)             
            rotationSpeed = 0.0f;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    cameraZoom -= (float)yoffset;
    if (cameraZoom < 1.0f)
        cameraZoom = 1.0f;
    if (cameraZoom > 150.0f)
        cameraZoom = 150.0f;
}