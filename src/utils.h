#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
    glm::mat4 mat;

  public:
    Transform() : mat(1.0f) {}

    void translate(const glm::vec3 &delta) { mat = glm::translate(mat, delta); }
    void scale(const glm::vec3 &factor) { mat = glm::scale(mat, factor); }
    void rotate(float angleDegrees, const glm::vec3 &axis) {
        mat = glm::rotate(mat, glm::radians(angleDegrees), axis);
    }

    void setPosition(const glm::vec3 &position) {
        // Extract current scale and rotation
        glm::vec3 currentScale = glm::vec3(glm::length(glm::vec3(mat[0])),
                                           glm::length(glm::vec3(mat[1])),
                                           glm::length(glm::vec3(mat[2])));
        glm::mat4 rotation = mat;
        rotation[0] /= currentScale.x;
        rotation[1] /= currentScale.y;
        rotation[2] /= currentScale.z;

        // Create new transform with new position but same rotation and scale
        mat = glm::translate(glm::mat4(1.0f), position) * rotation;
        mat[0] *= currentScale.x;
        mat[1] *= currentScale.y;
        mat[2] *= currentScale.z;
    }
    void setRotation(float angleDegrees, const glm::vec3 &axis) {
        // Extract current position and scale
        glm::vec3 currentPosition = glm::vec3(mat[3]);
        glm::vec3 currentScale = glm::vec3(glm::length(glm::vec3(mat[0])),
                                           glm::length(glm::vec3(mat[1])),
                                           glm::length(glm::vec3(mat[2])));

        // Create new transform with new rotation but same position and scale
        mat = glm::translate(glm::mat4(1.0f), currentPosition) *
              glm::rotate(glm::mat4(1.0f), glm::radians(angleDegrees), axis);
        mat[0] *= currentScale.x;
        mat[1] *= currentScale.y;
        mat[2] *= currentScale.z;
    }
    void setScale(const glm::vec3 &factor) {
        // Extract current position and rotation
        glm::vec3 currentPosition = glm::vec3(mat[3]);
        glm::mat4 rotation = mat;
        rotation[0] /= glm::length(glm::vec3(mat[0]));
        rotation[1] /= glm::length(glm::vec3(mat[1]));
        rotation[2] /= glm::length(glm::vec3(mat[2]));

        // Create new transform with new scale but same position and rotation
        mat = glm::translate(glm::mat4(1.0f), currentPosition) * rotation;
        mat[0] *= factor.x;
        mat[1] *= factor.y;
        mat[2] *= factor.z;
    }

    glm::vec3 getPosition() const { return glm::vec3(mat[3]); }
    glm::vec3 getScale() const {
        return glm::vec3(glm::length(glm::vec3(mat[0])),
                         glm::length(glm::vec3(mat[1])),
                         glm::length(glm::vec3(mat[2])));
    }
    glm::vec3 getRotation() const {
        // Extract rotation angles from the rotation part of the matrix
        glm::mat4 rotation = mat;
        rotation[0] /= glm::length(glm::vec3(mat[0]));
        rotation[1] /= glm::length(glm::vec3(mat[1]));
        rotation[2] /= glm::length(glm::vec3(mat[2]));

        float sy = sqrt(rotation[0][0] * rotation[0][0] +
                        rotation[1][0] * rotation[1][0]);
        bool singular = sy < 1e-6; // If

        float x, y, z;
        if (!singular) {
            x = atan2(rotation[2][1], rotation[2][2]);
            y = atan2(-rotation[2][0], sy);
            z = atan2(rotation[1][0], rotation[0][0]);
        } else {
            x = atan2(-rotation[1][2], rotation[1][1]);
            y = atan2(-rotation[2][0], sy);
            z = 0;
        }
        return glm::degrees(glm::vec3(x, y, z));
    }

    const glm::mat4 &getMatrix() const { return mat; }
};

#endif