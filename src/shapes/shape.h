#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>
#include <optional>
#include <memory>

#include "../mesh/mesh.h"
#include "../utils.h"
#include "../textures/texture.h"
#include "../shaders/shader_params.h"
#include "../globals.h"

struct Shape {
    const std::weak_ptr<Mesh> mesh; // Pointer to shared GPU data
    std::optional<glm::vec3> colorOverride;
    std::weak_ptr<Texture> baseColor;
    std::weak_ptr<Texture> roughnessMap;
    float roughness = 0.4f;
    float metallic = 0.0f;
    float specular = 0.5f;
    glm::vec3 minBounds, maxBounds;

    std::function<void()> onBoundsUpdated;

    Transform transform;

    Shape(const std::shared_ptr<Mesh> &m)
        : mesh(m), transform([this]() { onTransformUpdated(); }) {
        if (auto meshShared = mesh.lock()) {
            minBounds =
                glm::vec3(meshShared->minX, meshShared->minY, meshShared->minZ);
            maxBounds =
                glm::vec3(meshShared->maxX, meshShared->maxY, meshShared->maxZ);
        }
    }

    void updateBounds(const glm::mat4 &parentTransform) {
        auto meshShared = mesh.lock();
        if (!meshShared) {
            return;
        }
        glm::vec3 corners[8] = {
            {meshShared->minX, meshShared->minY, meshShared->minZ},
            {meshShared->maxX, meshShared->minY, meshShared->minZ},
            {meshShared->minX, meshShared->maxY, meshShared->minZ},
            {meshShared->maxX, meshShared->maxY, meshShared->minZ},
            {meshShared->minX, meshShared->minY, meshShared->maxZ},
            {meshShared->maxX, meshShared->minY, meshShared->maxZ},
            {meshShared->minX, meshShared->maxY, meshShared->maxZ},
            {meshShared->maxX, meshShared->maxY, meshShared->maxZ},
        };
        glm::vec3 transformedMin(FLT_MAX);
        glm::vec3 transformedMax(-FLT_MAX);
        for (const auto &corner : corners) {
            glm::vec4 transformedCorner = parentTransform *
                                          transform.getMatrix() *
                                          glm::vec4(corner, 1.0f);
            transformedMin =
                glm::min(transformedMin, glm::vec3(transformedCorner));
            transformedMax =
                glm::max(transformedMax, glm::vec3(transformedCorner));
        }
        minBounds = transformedMin;
        maxBounds = transformedMax;
    }

    void onTransformUpdated() {
        updateBounds(glm::mat4(1.0f));
        if (onBoundsUpdated) {
            onBoundsUpdated();
        }
    }

    void setOnBoundsUpdated(std::function<void()> callback) {
        onBoundsUpdated = std::move(callback);
    }

    void setColorOverride(const glm::vec3 &color) { colorOverride = color; }

    void bindTextureBaseColor(const std::shared_ptr<Texture> &texture) {
        baseColor = texture;
    }

    void bindTextureRoughnessMap(const std::shared_ptr<Texture> &texture) {
        roughnessMap = texture;
    }

    void setRoughness(float r) { roughness = r; }
    void setMetallic(float m) { metallic = m; }
    void setSpecular(float s) { specular = s; }

    void draw(Shader &shader, const glm::mat4 &parentTransform) const {
        draw(shader, parentTransform, GL_TRIANGLES);
    }

    void draw(Shader &shader, const glm::mat4 &parentTransform,
              GLenum primitiveMode) const {
        auto meshShared = mesh.lock();
        if (!meshShared) {
            return;
        }

        glUseProgram(shader.ID);
        const bool shouldRestoreColorAttrib = meshShared->hasColors;
        if (colorOverride.has_value()) {
            const glm::vec3 &c = colorOverride.value();
            glBindVertexArray(meshShared->VAO);
            glDisableVertexAttribArray(MeshAttrib::Color);
            glVertexAttrib3f(MeshAttrib::Color, c.r, c.g, c.b);
        }

        // Pass transform to shader
        unsigned int modelLoc = glGetUniformLocation(shader.ID, "model");
        glm::mat4 model = parentTransform * transform.getMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw the shared mesh
        glBindVertexArray(meshShared->VAO);

        const bool hasBase = !baseColor.expired();
        const bool hasRough = !roughnessMap.expired();
        shader.setBool("hasBaseColorMap", hasBase);
        shader.setBool("hasRoughnessMap", hasRough);

        if (auto texShared = baseColor.lock()) {
            texShared->bind(BASE_COLOR_TEXTURE_UNIT);
        }
        if (auto roughTexShared = roughnessMap.lock()) {
            roughTexShared->bind(ROUGHNESS_TEXTURE_UNIT);
        }
        shader_utils::set_blinn_phong_material_uniforms(shader, roughness,
                                                        metallic, specular);

        gameStateData.addDrawCall();
        glDrawElements(primitiveMode, meshShared->indexCount, GL_UNSIGNED_INT,
                       0);
        if (auto texShared = baseColor.lock()) {
            texShared->unbind(BASE_COLOR_TEXTURE_UNIT);
        }
        if (auto roughTexShared = roughnessMap.lock()) {
            roughTexShared->unbind(ROUGHNESS_TEXTURE_UNIT);
        }

        if (colorOverride.has_value() && shouldRestoreColorAttrib) {
            glBindVertexArray(meshShared->VAO);
            glEnableVertexAttribArray(MeshAttrib::Color);
        }
    }

    float minX() { return minBounds.x; }
    float minY() { return minBounds.y; }
    float minZ() { return minBounds.z; }
    float maxX() { return maxBounds.x; }
    float maxY() { return maxBounds.y; }
    float maxZ() { return maxBounds.z; }
};
#endif