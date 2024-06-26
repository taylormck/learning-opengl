#ifndef MESH_H
#define MESH_H

#include <vector>

#include "shader.hpp"
#include "texture.hpp"
#include "vertex.hpp"

#include "openGLCommon.hpp"

class Mesh {
private:
    GLuint VAO, VBO, EBO;

    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(Vertex), &Vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(GLuint), &Indices[0], GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

        // Vertex Texture Coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TextureCoordinates));

        // Vertex Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

        // Vertex BitTangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, BitTangent));

        glBindVertexArray(0);
    }

public:
    std::vector<Vertex> Vertices;
    std::vector<GLuint> Indices;
    std::vector<Texture> Textures;
    GLfloat Shininess;

    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, float shininess) :
        Vertices(vertices), Indices(indices), Textures(textures), Shininess(shininess) {
        setupMesh();
    }

    void Draw(Shader &shader) const {
        unsigned int diffuseNr = 0;
        unsigned int specularNr = 0;
        unsigned int normalNr = 0;

        for (unsigned int i = 0; i < Textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);

            std::string number;
            std::string name = Textures[i].Type;

            if (name == "texture_diffuse") {
                name += std::to_string(diffuseNr);
                ++diffuseNr;
            } else if (name == "texture_specular") {
                name += std::to_string(specularNr);
                ++specularNr;
            } else if (name == "texture_normal") {
                name += std::to_string(normalNr);
                ++normalNr;
            }

            shader.setInt("material." + name, i);

            glBindTexture(GL_TEXTURE_2D, Textures[i].ID);
        }

        shader.setFloat("material.shininess", Shininess);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }
};

#endif
