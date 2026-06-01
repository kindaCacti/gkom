#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define Y_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define Z_UP glm::vec3(0.0f, 0.0f, 1.0f)
class Camera {
    glm::vec3 position = glm::vec3(0.0f, 3.0f, 0.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = Z_UP;
    float fov = 60.0f;
    float aspectRatio = 800.0f / 600.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);

    // Orbit camera parameters (around the current target).
    float orbitYawDeg = 0.f;
    float minYawDeg = CONTROLS_MODE == THIRD_PERSON ? -360.f : -10.f;
    float maxYawDeg = CONTROLS_MODE == THIRD_PERSON ? 360.f : 10.f;
    float orbitPitchDeg = 0.f;
    float minPitchDeg = CONTROLS_MODE == THIRD_PERSON ? 0.f : 70.f;
    float maxPitchDeg = 80.f;
    float orbitRadius = 5.f;
    float orbitSensitivity = 0.05f;
    bool orbitEnabled = false;
    bool orbitFirstMouse = true;
    double orbitLastX = 0.0;
    double orbitLastY = 0.0;

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

    void setOrbitSensitivity(float sensitivity) {
        orbitSensitivity = sensitivity;
    }

    void initOrbitForTarget(const glm::vec3 &targetPos) {
        orbitEnabled = true;
        orbitFirstMouse = true;

        target = targetPos;
        const glm::vec3 delta = position - targetPos;
        orbitRadius = glm::length(delta);
        if (orbitRadius < 1e-4f) {
            orbitRadius = 1.f;
        }
        orbitYawDeg = glm::degrees(std::atan2(delta.y, delta.x));
        orbitPitchDeg = glm::degrees(
            std::asin(std::clamp(delta.z / orbitRadius, -1.f, 1.f)));
        recalculateViewMatrix();
    }

    // Mouse move handler: updates yaw/pitch for orbit camera.
    void onMouseMove(double xpos, double ypos) {
        if (!orbitEnabled) {
            return;
        }
        if (orbitFirstMouse) {
            orbitLastX = xpos;
            orbitLastY = ypos;
            orbitFirstMouse = false;
            return;
        }

        const double dx = orbitLastX - xpos;
        const double dy = ypos - orbitLastY;
        orbitLastX = xpos;
        orbitLastY = ypos;

        orbitYawDeg += static_cast<float>(dx) * orbitSensitivity;
        orbitYawDeg = std::clamp(orbitYawDeg, minYawDeg, maxYawDeg);
        orbitYawDeg = std::fmod(orbitYawDeg, 360.f); // Wrap yaw to [0, 360)
        orbitPitchDeg += static_cast<float>(dy) * orbitSensitivity;
        orbitPitchDeg = std::clamp(orbitPitchDeg, minPitchDeg, maxPitchDeg);
    }

    // Orbit around the given target position (Z-up).
    void orbitAround(const glm::vec3 &targetPos) {
        if (!orbitEnabled) {
            setTarget(glm::vec3(targetPos));
            return;
        }

        const float yaw = glm::radians(orbitYawDeg);
        const float pitch = glm::radians(orbitPitchDeg);
        const float cosPitch = std::cos(pitch);
        const glm::vec3 offset(orbitRadius * cosPitch * std::cos(yaw),
                               orbitRadius * cosPitch * std::sin(yaw),
                               orbitRadius * std::sin(pitch));

        setTarget(glm::vec3(targetPos));
        setPosition(glm::vec3(targetPos + offset));
    }

    const glm::mat4 &getMatrix() const { return viewProjectionMatrix; }
    const glm::vec3 &getPosition() const { return position; }
    const float getYaw() const { return orbitYawDeg; }
    const float getPitch() const { return orbitPitchDeg; }
    const glm::vec3 getXYDirection() const {
        glm::vec3 dir = target - position;
        dir.z = 0.f;
        return glm::normalize(dir);
    }
    const glm::mat4 &getViewMatrix() const { return viewMatrix; }
    const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }
};

#endif // CAMERA_H