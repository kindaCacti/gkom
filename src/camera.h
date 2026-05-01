#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define Y_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define Z_UP glm::vec3(0.0f, 0.0f, 1.0f)
class Camera {
    glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = Z_UP;
    float fov = 45.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);

    void recalculateViewProjectionMatrix() {
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }

    void recalculateViewMatrix() {
        viewMatrix = glm::lookAt(position, target, up);
        recalculateViewProjectionMatrix();
    }

    void recalculateProjectionMatrix() {
        projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio,
                                            nearPlane, farPlane);
        recalculateViewProjectionMatrix();
    }

  public:
    Camera() {
        recalculateViewMatrix();
        recalculateProjectionMatrix();
    }

    void setPosition(const glm::vec3 &&pos) {
        position = std::move(pos);
        recalculateViewMatrix();
    }

    void setTarget(const glm::vec3 &&tgt) {
        target = std::move(tgt);
        recalculateViewMatrix();
    }

    void setUp(const glm::vec3 &&upVec) {
        up = std::move(upVec);
        recalculateViewMatrix();
    }

    void setFOV(float fovDegrees) {
        fov = fovDegrees;
        recalculateProjectionMatrix();
    }

    void setAspectRatio(float ratio) {
        aspectRatio = ratio;
        recalculateProjectionMatrix();
    }

    void setClippingPlanes(float nearP, float farP) {
        nearPlane = nearP;
        farPlane = farP;
        recalculateProjectionMatrix();
    }

    const glm::mat4 &getMatrix() const { return viewProjectionMatrix; }
    const glm::vec3 &getPosition() const { return position; }
};

#endif // CAMERA_H