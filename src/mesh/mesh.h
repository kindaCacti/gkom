#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <cstdint>

#include <glad/glad.h>

namespace MeshAttrib {
constexpr unsigned int Position = 0;
constexpr unsigned int Normal = 1;
constexpr unsigned int Color = 2;
constexpr unsigned int TexCoord = 3;
} // namespace MeshAttrib

struct Mesh {
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    bool hasTexcoords = false;
    bool hasNormals = false;
    bool hasColors = false;
    float minX = 0.f, maxX = 0.f, minY = 0.f, maxY = 0.f, minZ = 0.f, maxZ = 0.f;

    Mesh(const Mesh &) = delete;
    Mesh &operator=(const Mesh &) = delete;

    Mesh(Mesh &&other) noexcept
        : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
          indexCount(other.indexCount), hasTexcoords(other.hasTexcoords),
          hasNormals(other.hasNormals), hasColors(other.hasColors),
          minX(other.minX), maxX(other.maxX), minY(other.minY), maxY(other.maxY), minZ(other.minZ), maxZ(other.maxZ) {
        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.indexCount = 0;
        other.hasTexcoords = false;
        other.hasNormals = false;
        other.hasColors = false;
    }

    Mesh &operator=(Mesh &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
        }
        if (EBO != 0) {
            glDeleteBuffers(1, &EBO);
        }

        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;
        indexCount = other.indexCount;
        hasTexcoords = other.hasTexcoords;
        hasNormals = other.hasNormals;
        hasColors = other.hasColors;
        minX = other.minX;
        maxX = other.maxX;
        minY = other.minY;
        maxY = other.maxY;
        minZ = other.minZ;
        maxZ = other.maxZ;

        other.VAO = 0;
        other.VBO = 0;
        other.EBO = 0;
        other.indexCount = 0;
        other.hasTexcoords = false;
        other.hasNormals = false;
        other.hasColors = false;
        return *this;
    }

    Mesh(const std::vector<float> &vertices,
         const std::vector<unsigned int> &indices, bool hasNormals,
         bool hasColors, bool hasTexcoords, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
        : indexCount(static_cast<unsigned int>(indices.size())),
          hasNormals(hasNormals), hasColors(hasColors),
          hasTexcoords(hasTexcoords), minX(minX), maxX(maxX), minY(minY), maxY(maxY), minZ(minZ), maxZ(maxZ) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
            indices.data(), GL_STATIC_DRAW);

        // Packed order: position, normal, color, texCoord
        const int strideFloats = 3 + (hasNormals ? 3 : 0) +
                                 (hasColors ? 3 : 0) + (hasTexcoords ? 2 : 0);
        const int strideBytes = strideFloats * static_cast<int>(sizeof(float));

        // location: position
        glVertexAttribPointer(MeshAttrib::Position, 3, GL_FLOAT, GL_FALSE,
                              strideBytes, (void *)0);
        glEnableVertexAttribArray(MeshAttrib::Position);

        int offsetFloats = 3;
        if (hasNormals) {
            glVertexAttribPointer(
                MeshAttrib::Normal, 3, GL_FLOAT, GL_FALSE, strideBytes,
                (void *)(static_cast<std::intptr_t>(offsetFloats) *
                         sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::Normal);
            offsetFloats += 3;
        }
        if (hasColors) {
            glVertexAttribPointer(
                MeshAttrib::Color, 3, GL_FLOAT, GL_FALSE, strideBytes,
                (void *)(static_cast<std::intptr_t>(offsetFloats) *
                         sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::Color);
            offsetFloats += 3;
        }
        if (hasTexcoords) {
            glVertexAttribPointer(
                MeshAttrib::TexCoord, 2, GL_FLOAT, GL_FALSE, strideBytes,
                (void *)(static_cast<std::intptr_t>(offsetFloats) *
                         sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::TexCoord);
            offsetFloats += 2;
        }
    }

    ~Mesh() {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
        }
        if (EBO != 0) {
            glDeleteBuffers(1, &EBO);
        }
    }

};
#endif