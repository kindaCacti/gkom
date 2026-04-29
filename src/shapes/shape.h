#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../mesh/mesh.h"

class Shape {
  public:
    glm::mat4 transform = glm::mat4(1.0f);
    const Mesh *mesh; // Pointer to shared GPU data

    Shape(const Mesh *m) : mesh(m) {}

    void draw(unsigned int shaderProgram) const {
        // Pass transform to shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));

        // Draw the shared mesh
        glBindVertexArray(mesh->VAO);
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    }
};
#endif