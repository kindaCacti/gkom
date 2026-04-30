#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../mesh/mesh.h"
#include "../utils.h"

class Shape {
    const Mesh *mesh; // Pointer to shared GPU data

  public:
    Transform transform;

    Shape(const Mesh *m) : mesh(m) {}

    void draw(unsigned int shaderProgram,
              const glm::mat4 &parentTransform) const {
        // Pass transform to shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 model = parentTransform * transform.getMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw the shared mesh
        glBindVertexArray(mesh->VAO);
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    }
};
#endif