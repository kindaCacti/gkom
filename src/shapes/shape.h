#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <optional>

#include "../mesh/mesh.h"
#include "../utils.h"

class Shape {
    const Mesh *mesh; // Pointer to shared GPU data
    std::optional<glm::vec3> colorOverride;

  public:
    Transform transform;

    Shape(const Mesh *m) : mesh(m) {}

    void setColorOverride(const glm::vec3 &color) { colorOverride = color; }

    void draw(unsigned int shaderProgram,
              const glm::mat4 &parentTransform) const {
        const bool shouldRestoreColorAttrib = mesh->hasColors;
        if (colorOverride.has_value()) {
            const glm::vec3 &c = colorOverride.value();
            glBindVertexArray(mesh->VAO);
            glDisableVertexAttribArray(MeshAttrib::Color);
            glVertexAttrib3f(MeshAttrib::Color, c.r, c.g, c.b);
        }

        // Pass transform to shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 model = parentTransform * transform.getMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw the shared mesh
        glBindVertexArray(mesh->VAO);
        glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

        if (colorOverride.has_value() && shouldRestoreColorAttrib) {
            glBindVertexArray(mesh->VAO);
            glEnableVertexAttribArray(MeshAttrib::Color);
        }
    }
};
#endif