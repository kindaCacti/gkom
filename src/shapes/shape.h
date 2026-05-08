#ifndef SHAPES_H
#define SHAPES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <optional>
#include <memory>

#include "../mesh/mesh.h"
#include "../utils.h"
#include "../textures/texture.h"
#include "../shaders/shader_params.h"

class Shape {
    const std::weak_ptr<Mesh> mesh; // Pointer to shared GPU data
    std::optional<glm::vec3> colorOverride;
    std::weak_ptr<Texture> diffuseTexture;

  public:
    Transform transform;

    Shape(const std::shared_ptr<Mesh> &m) : mesh(m) {}

    void setColorOverride(const glm::vec3 &color) { colorOverride = color; }

    void bindDiffuseTexture(const std::shared_ptr<Texture> &texture) {
        diffuseTexture = texture;
    }

    void draw(unsigned int shaderProgram,
              const glm::mat4 &parentTransform) const {
        auto meshShared = mesh.lock();
        if (!meshShared) {
            return;
        }

        const bool shouldRestoreColorAttrib = meshShared->hasColors;
        if (colorOverride.has_value()) {
            const glm::vec3 &c = colorOverride.value();
            glBindVertexArray(meshShared->VAO);
            glDisableVertexAttribArray(MeshAttrib::Color);
            glVertexAttrib3f(MeshAttrib::Color, c.r, c.g, c.b);
        }

        // Pass transform to shader
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glm::mat4 model = parentTransform * transform.getMatrix();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Draw the shared mesh
        glBindVertexArray(meshShared->VAO);
        if (auto texShared = diffuseTexture.lock()) {
            texShared->bind(DIFFUSE_TEXTURE_UNIT);
        }
        glDrawElements(GL_TRIANGLES, meshShared->indexCount, GL_UNSIGNED_INT,
                       0);
        if (auto texShared = diffuseTexture.lock()) {
            texShared->unbind();
        }

        if (colorOverride.has_value() && shouldRestoreColorAttrib) {
            glBindVertexArray(meshShared->VAO);
            glEnableVertexAttribArray(MeshAttrib::Color);
        }
    }

    float minX() { return mesh.lock()->minX; }
    float minY() { return mesh.lock()->minY; }
    float minZ() { return mesh.lock()->minZ; }
    float maxX() { return mesh.lock()->maxX; }
    float maxY() { return mesh.lock()->maxY; }
    float maxZ() { return mesh.lock()->maxZ; }
};
#endif