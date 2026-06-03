#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include <map>
#include <memory>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../mesh/mesh.h"
#include "../mesh/mesh_loader.h"
#include "../utils.h"
#include "shape.h"

class ShapeFactory {
  private:
    // This stores the actual GPU data so it stays in memory
    std::map<std::string, std::shared_ptr<Mesh>> _meshCache;
    std::map<std::string, Transform> _transformCache;

    void _loadCubeMesh() {
        float vertices[] = {
            // position          // color
            0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, // bottom left
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // top
            -0.5f, 0.5f,  0.5f,  0.5f, 0.5f, 0.0f, // top
            0.5f,  -0.5f, -0.5f, 1.0f, 0.5f, 0.0f, // bottom right
            -0.5f, -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, // bottom left
            0.5f,  0.5f,  -0.5f, 0.0f, 0.5f, 0.5f, // top
            -0.5f, 0.5f,  -0.5f, 0.5f, 0.0f, 1.0f, // top
        };

        unsigned int indices[] = {
            0, 1, 2, 1, 3, 2, // Front
            4, 5, 6, 5, 7, 6, // Back
            0, 4, 1, 4, 5, 1, // Bottom
            2, 6, 3, 6, 7, 3, // Top
            0, 2, 4, 2, 6, 4, // Right
            1, 3, 5, 3, 7, 5  // Left
        };

        auto m = std::make_shared<Mesh>(
            std::vector<float>(vertices, vertices + 6 * 8),
            std::vector<unsigned int>(indices, indices + 36), false, true,
            false, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f);

        _meshCache["cube"] = m;
    }

    void _loadWireCubeMesh(const std::string &name) {
        // Unit cube centered at origin in [-0.5, 0.5]^3, with line indices.
        // Vertex layout: position + normal (to satisfy default shader inputs).
        const std::vector<float> vertices = {
            // position              // normal
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // 0
            0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // 1
            0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, // 2
            -0.5f, 0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, // 3
            -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // 4
            0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // 5
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // 6
            -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, 1.0f  // 7
        };
        const std::vector<unsigned int> indices = {
            0, 1, 1, 2, 2, 3, 3, 0, // bottom loop
            4, 5, 5, 6, 6, 7, 7, 4, // top loop
            0, 4, 1, 5, 2, 6, 3, 7  // vertical edges
        };

        auto m = std::make_shared<Mesh>(vertices, indices,
                                        /*hasNormals=*/true,
                                        /*hasColors=*/false,
                                        /*hasTexcoords=*/false, -0.5f, 0.5f,
                                        -0.5f, 0.5f, -0.5f, 0.5f,
                                        /*hasRoughness=*/false);

        _meshCache[name] = m;
    }

    void _loadWireCylinderMesh(const std::string &name, int segments = 24) {
        if (segments < 3)
            segments = 3;

        // Unit cylinder centered at origin.
        // Radius = 0.5 in X/Y, height = 1.0 along Z (-0.5..0.5).
        // Vertex layout: position + normal (to satisfy default shader inputs).
        std::vector<float> vertices;
        vertices.reserve(static_cast<size_t>(segments) * 2 * 6);

        std::vector<unsigned int> indices;
        indices.reserve(static_cast<size_t>(segments) * 6);

        const float pi = 3.14159265358979323846f;

        for (int i = 0; i < segments; ++i) {
            const float a =
                2.0f * pi *
                (static_cast<float>(i) / static_cast<float>(segments));
            const float x = 0.5f * std::cos(a);
            const float y = 0.5f * std::sin(a);

            // bottom vertex
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(-0.5f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);

            // top vertex
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.5f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
        }

        for (int i = 0; i < segments; ++i) {
            const int next = (i + 1) % segments;
            const unsigned int bi = static_cast<unsigned int>(2 * i);
            const unsigned int ti = static_cast<unsigned int>(2 * i + 1);
            const unsigned int bnext = static_cast<unsigned int>(2 * next);
            const unsigned int tnext = static_cast<unsigned int>(2 * next + 1);

            // bottom loop
            indices.push_back(bi);
            indices.push_back(bnext);

            // top loop
            indices.push_back(ti);
            indices.push_back(tnext);

            // vertical edge
            indices.push_back(bi);
            indices.push_back(ti);
        }

        auto m = std::make_shared<Mesh>(vertices, indices,
                                        /*hasNormals=*/true,
                                        /*hasColors=*/false,
                                        /*hasTexcoords=*/false, -0.5f, 0.5f,
                                        -0.5f, 0.5f, -0.5f, 0.5f,
                                        /*hasRoughness=*/false);

        _meshCache[name] = m;
    }

  public:
    ShapeFactory() {
        _loadCubeMesh();
        _transformCache["cube"] = Transform();
    }

    void registerTransform(const std::string &name,
                           const Transform &transform) {
        _transformCache[name] = transform;
    }

    void registerMesh(const std::shared_ptr<Mesh> &mesh,
                      const std::string &name) {
        _meshCache[name] = mesh;
    }

    void registerCube() { _loadCubeMesh(); }

    void registerWireCube(const std::string &name = "hitbox_cube") {
        _loadWireCubeMesh(name);
        _transformCache[name] = Transform();
    }

    void registerWireCylinder(const std::string &name = "hitbox_cylinder") {
        _loadWireCylinderMesh(name);
        _transformCache[name] = Transform();
    }

    void registerMesh(const std::string &path, const std::string &name,
                      std::optional<glm::vec3> color = std::nullopt) {
        auto meshOpt = mesh_loader::load_obj(path, color);
        if (meshOpt.has_value()) {
            _meshCache[name] =
                std::make_shared<Mesh>(std::move(meshOpt.value()));
        }
    }

    std::unique_ptr<Shape>
    createShape(const std::string &name,
                std::optional<glm::vec3> colorOverride = std::nullopt) {
        auto it = _meshCache.find(name);
        if (it != _meshCache.end()) {
            auto newShape = std::make_unique<Shape>(it->second);
            if (colorOverride.has_value()) {
                newShape->setColorOverride(colorOverride.value());
            }
            if (auto transform = _transformCache.find(name);
                transform != _transformCache.end()) {
                newShape->transform.setMatrix(transform->second.getMatrix());
            }
            return newShape;
        }
        return nullptr;
    }

    // std::unique_ptr<Shape> createCube(glm::vec3 position) {
    //     auto it = _meshCache.find("cube");
    //     if (it != _meshCache.end()) {
    //         auto newShape = std::make_unique<Shape>(&it->second);
    //         newShape->transform.translate(position);
    //         return newShape;
    //     }
    //     return nullptr;
    // }

    // Clean up GPU resources on destruction
    ~ShapeFactory() {}
};

#endif