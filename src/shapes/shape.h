#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../mesh/mesh.h"
#include "../utils.h"

class Shape {
    const Mesh *mesh; // Pointer to shared GPU data

  public:
    Transform transform; // Local transform

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

    void reset_transform() { transform = glm::mat4(1.f); }
    void translate(const glm::vec3 &delta) {
        transform = glm::translate(transform, delta);
    }
    void scale(const glm::vec3 &factor) {
        transform = glm::scale(transform, factor);
    }
    void rotate(float angleDegrees, const glm::vec3 &axis) {
        transform = glm::rotate(transform, glm::radians(angleDegrees), axis);
    }
    void rotate(const glm::vec3 &eulerAngles) {
        auto rotMatrix = glm::mat4_cast(glm::quat(eulerAngles));
        transform = transform * rotMatrix;
    }

    void setPosition(const glm::vec3 &position) {
        // Extract current scale and rotation
        glm::vec3 currentScale =
            glm::vec3(glm::length(glm::vec3(transform[0])),
                      glm::length(glm::vec3(transform[1])),
                      glm::length(glm::vec3(transform[2])));
        glm::mat4 rotation = transform;
        rotation[0] /= currentScale.x;
        rotation[1] /= currentScale.y;
        rotation[2] /= currentScale.z;

        // Create new transform with new position but same rotation and scale
        transform = glm::translate(glm::mat4(1.0f), position) * rotation;
        transform[0] *= currentScale.x;
        transform[1] *= currentScale.y;
        transform[2] *= currentScale.z;
    }
    void setRotation(float angleDegrees, const glm::vec3 &axis) {
        // Extract current position and scale
        glm::vec3 currentPosition = glm::vec3(transform[3]);
        glm::vec3 currentScale =
            glm::vec3(glm::length(glm::vec3(transform[0])),
                      glm::length(glm::vec3(transform[1])),
                      glm::length(glm::vec3(transform[2])));

        // Create new transform with new rotation but same position and scale
        transform =
            glm::translate(glm::mat4(1.0f), currentPosition) *
            glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), axis);
        transform[0] *= currentScale.x;
        transform[1] *= currentScale.y;
        transform[2] *= currentScale.z;
    }
    void setScale(const glm::vec3 &factor) {
        // Extract current position and rotation
        glm::vec3 currentPosition = glm::vec3(transform[3]);
        glm::mat4 rotation = transform;
        rotation[0] /= glm::length(glm::vec3(transform[0]));
        rotation[1] /= glm::length(glm::vec3(transform[1]));
        rotation[2] /= glm::length(glm::vec3(transform[2]));

        // Create new transform with new scale but same position and rotation
        transform = glm::translate(glm::mat4(1.0f), currentPosition) * rotation;
        transform[0] *= factor.x;
        transform[1] *= factor.y;
        transform[2] *= factor.z;
    }
};
#endif