#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../texture/Texture2D.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec2 TexCoords;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);

    void Draw();
private:
    unsigned int VBO, EBO;
    void setupMesh();
};

#endif
