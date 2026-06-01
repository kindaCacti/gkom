#ifndef BULLET_BUFFER_H
#define BULLET_BUFFER_H

#include <vector>
#include <glm/glm.hpp>
#include "./entities/bullet.h"
#include "defines.h"
#include "globals.h"

class BulletBuffer {
    std::vector<std::shared_ptr<Bullet>> _elements;
    size_t _activeCount = 0;
    unsigned int instancedVBO;
    glm::mat4 *modelTransforms = nullptr;
    unsigned int meshVAO;
    unsigned int meshIndecesCount;

  public:
    BulletBuffer() = default;
    BulletBuffer(const BulletBuffer &) = default;
    BulletBuffer(BulletBuffer &&) = default;
    BulletBuffer &operator=(const BulletBuffer &) = default;
    BulletBuffer &operator=(BulletBuffer &&) = default;
    ~BulletBuffer() { delete[] modelTransforms; }

    size_t activeElementCount() const { return _activeCount; }

    std::shared_ptr<Bullet> createBullet(std::unique_ptr<Shape> &&shape,
                                         float speed, glm::vec3 &direction) {
        if (_activeCount == _elements.size()) {
            _elements.push_back(
                std::make_shared<Bullet>(std::move(shape), speed, direction));
            return _elements[_activeCount++];
        }

        _elements[_activeCount]->setShape(std::move(shape));
        _elements[_activeCount]->setSpeed(speed);
        _elements[_activeCount]->setDirection(direction);

        return _elements[_activeCount++];
    }

    void deactivateElement(size_t position) {
        if (_activeCount == 0 || position >= _activeCount)
            return;
        --_activeCount;
        if (position != _activeCount) {
            std::swap(_elements[position], _elements[_activeCount]);
        }
    }

    void moveRemoveActiveElements(float deltaTime, glm::vec3 &&target) {
        moveRemoveActiveElements(deltaTime, target);
    }

    void moveRemoveActiveElements(float deltaTime, glm::vec3 &target) {
        for (size_t i = 0; i < _activeCount;) {
            _elements[i]->step(deltaTime, target);
            auto bulletPosition = _elements[i]->get_pos();
            if (bulletPosition.x * bulletPosition.x +
                    bulletPosition.z * bulletPosition.z >=
                AREA_RADIUS_SQ) {
                deactivateElement(i);
                // do NOT increment i: the bullet swapped into slot i still
                // needs to be processed this frame.
                continue;
            }
            ++i;
        }
    }

    void drawActiveElements(Shader &shader) {
        for (size_t i = 0; i < _activeCount; i++) {
            _elements[i]->draw(shader);
        }
    }

    std::vector<int>
    checkActiveBulletCollision(HitboxedDrawableEntity *target) {
        if (_activeCount == 0)
            return {};
        // We assume that each bullet's bounding box is the same size, so we can
        // precompute the sum of the containing sphere radii.
        const float max_dist = _elements[0]->containingSphereRadius() +
                               target->containingSphereRadius();
        const float md2 =
            max_dist * max_dist; // Compare squared distances to avoid sqrt
        std::vector<int> collisions;
        for (size_t i = 0; i < _activeCount; i++) {
            // Cheap bb reject before expensive intersection calculation.
            const glm::vec3 d = _elements[i]->get_pos() - target->get_pos();
            const float d2 = d.x * d.x + d.y * d.y + d.z * d.z;
            if (d2 > md2) // No collision possible
                continue;

            // Precise check:
            if (_elements[i]->intersects(target))
                collisions.push_back(static_cast<int>(i));
        }
        return collisions;
    }

    void setupInstancedDrawing(unsigned int bulletMeshVAO,
                               unsigned int _meshIndecesCount) {
        modelTransforms = new glm::mat4[MAX_BULLETS];
        glGenBuffers(1, &instancedVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);

        // FIX 1: Changed GL_STATIC_DRAW to GL_DYNAMIC_DRAW
        // FIX 2: Passed nullptr instead of garbage array pointer to safely
        // reserve space
        glBufferData(GL_ARRAY_BUFFER, MAX_BULLETS * sizeof(glm::mat4), nullptr,
                     GL_DYNAMIC_DRAW);

        meshVAO = bulletMeshVAO;
        meshIndecesCount = _meshIndecesCount;

        glBindVertexArray(meshVAO);

        std::size_t vec4Size = sizeof(glm::vec4);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                              (void *)0);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                              (void *)(1 * vec4Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                              (void *)(2 * vec4Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size,
                              (void *)(3 * vec4Size));

        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Clean up state
    }

    void drawActiveInstanced(Shader &shader) {
        gameStateData.addDrawCall();
        size_t count = activeElementCount();
        if (count > MAX_BULLETS)
            count = MAX_BULLETS;
        if (count == 0)
            return; // Nothing to draw!

        // 1. Gather the latest transform matrices on the CPU
        for (size_t i = 0; i < count; i++) {
            modelTransforms[i] =
                _elements[i]->getTransformMatrixWithShapeTransform();
        }

        // 2. Bind the VBO and upload the fresh data to the GPU
        // (You must bind the VBO here to tell OpenGL WHICH buffer to copy data
        // into)
        glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0, // Offset from start of buffer
                        count * sizeof(glm::mat4), // Size of data to upload
                        &modelTransforms[0]        // Pointer to CPU data
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Safe to unbind VBO now

        // 3. Render using the VAO
        glBindVertexArray(meshVAO);

        glDrawElementsInstanced(GL_TRIANGLES, meshIndecesCount, GL_UNSIGNED_INT,
                                0,
                                count // Match the exact amount we just uploaded
        );

        glBindVertexArray(0);
    }
};

#endif