#ifndef MESH_H
#define MESH_H

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

    static void setupBuffers(const std::vector<float> &vertices,
                            const std::vector<unsigned int> &indices, Mesh &mesh,
                            bool hasNormals, bool hasColors, bool hasTexcoords) {
        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                indices.data(), GL_STATIC_DRAW);

        mesh.hasNormals = hasNormals;
        mesh.hasColors = hasColors;
        mesh.hasTexcoords = hasTexcoords;

        // Packed order: position, normal, color, texCoord
        const int strideFloats = 3 + (hasNormals ? 3 : 0) + (hasColors ? 3 : 0) +
                    (hasTexcoords ? 2 : 0);
        const int strideBytes = strideFloats * static_cast<int>(sizeof(float));

        // location: position
        glVertexAttribPointer(MeshAttrib::Position, 3, GL_FLOAT, GL_FALSE, strideBytes,
                    (void *)0);
        glEnableVertexAttribArray(MeshAttrib::Position);

        int offsetFloats = 3;
        if (hasNormals) {
            glVertexAttribPointer(MeshAttrib::Normal, 3, GL_FLOAT, GL_FALSE, strideBytes,
                        (void *)(static_cast<std::intptr_t>(offsetFloats) * sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::Normal);
            offsetFloats += 3;
        }
        if (hasColors) {
            glVertexAttribPointer(MeshAttrib::Color, 3, GL_FLOAT, GL_FALSE, strideBytes,
                        (void *)(static_cast<std::intptr_t>(offsetFloats) * sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::Color);
            offsetFloats += 3;
        }
        if (hasTexcoords) {
            glVertexAttribPointer(MeshAttrib::TexCoord, 2, GL_FLOAT, GL_FALSE, strideBytes,
                        (void *)(static_cast<std::intptr_t>(offsetFloats) * sizeof(float)));
            glEnableVertexAttribArray(MeshAttrib::TexCoord);
            offsetFloats += 2;
        }
    }
};

#endif