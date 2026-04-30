#ifndef SHAPE_FACTORY_H
#define SHAPE_FACTORY_H

#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../mesh/mesh.h"
#include "shape.h"

class ShapeFactory {
  private:
    // This stores the actual GPU data so it stays in memory
    std::map<std::string, Mesh> _meshCache;

    void _loadCubeMesh() {
        Mesh cube;
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
        cube.indexCount = 36;

        Mesh::setupBuffers(std::vector<float>(vertices, vertices + 6 * 8),
                           std::vector<unsigned int>(indices, indices + 36),
                           cube, false, true, false);

        _meshCache["cube"] = cube;
    }

  public:
    ShapeFactory() {
        _loadCubeMesh(); // Pre-load common shapes
    }

    void registerMesh(const Mesh &mesh, const std::string &name) {
        _meshCache[name] = mesh;
    }

    std::unique_ptr<Shape> createShape(const std::string &name) {
        auto it = _meshCache.find(name);
        if (it != _meshCache.end()) {
            auto newShape = std::make_unique<Shape>(&it->second);
            return newShape;
        }
        return nullptr;
    }

    std::unique_ptr<Shape> createCube(glm::vec3 position) {
        auto it = _meshCache.find("cube");
        if (it != _meshCache.end()) {
            auto newShape = std::make_unique<Shape>(&it->second);
            newShape->transform = glm::translate(glm::mat4(1.0f), position);
            return newShape;
        }
        return nullptr;
    }

    // Clean up GPU resources on destruction
    ~ShapeFactory() {
        for (auto &pair : _meshCache) {
            glDeleteVertexArrays(1, &pair.second.VAO);
            glDeleteBuffers(1, &pair.second.VBO);
        }
    }
};

#endif