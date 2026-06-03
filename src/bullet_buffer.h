#ifndef BULLET_BUFFER_H
#define BULLET_BUFFER_H

#include <vector>
#include <glm/glm.hpp>
#include "./entities/bullet.h"
#include "defines.h"
#include "globals.h"

class BulletBuffer {
  private:
    struct InstanceAffine {
        glm::vec3 col0;
        glm::vec3 col1;
        glm::vec3 col2;
        glm::vec3 col3; // translation
    };

    std::vector<std::shared_ptr<Bullet>> _elements;
    size_t _activeCount = 0;
    unsigned int instancedVBO;
    InstanceAffine *instanceTransforms = nullptr;
    unsigned int meshVAO;
    unsigned int meshIndecesCount;

    struct TargetInfo {
        HitboxedDrawableEntity *t = nullptr;
        glm::vec3 center = glm::vec3(0.0f);
        float md2 = 0.0f;
    };

  public:
    BulletBuffer() = default;
    BulletBuffer(const BulletBuffer &) = default;
    BulletBuffer(BulletBuffer &&) = default;
    BulletBuffer &operator=(const BulletBuffer &) = default;
    BulletBuffer &operator=(BulletBuffer &&) = default;
    ~BulletBuffer() { delete[] instanceTransforms; }

    size_t activeElementCount() const { return _activeCount; }

    void clearBuffer() { _activeCount = 0; }

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

    void drawActiveHitboxes(Shader &shader, const Shape &boxHitboxShape,
                            const Shape &cylinderHitboxShape) {
        for (size_t i = 0; i < _activeCount; i++) {
            _elements[i]->drawHitbox(shader, boxHitboxShape,
                                     cylinderHitboxShape);
        }
    }
    int expensiveCollisionChecks = 0;
    int cheapCollisionChecks = 0;

    void resetCollisionCounters() {
        expensiveCollisionChecks = 0;
        cheapCollisionChecks = 0;
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
        const glm::vec3 targetCenter = target->hitboxCenterWorld();
        for (size_t i = 0; i < _activeCount; i++) {
            // Cheap bb reject before expensive intersection calculation.
            ++cheapCollisionChecks;
            const glm::vec3 d =
                _elements[i]->hitboxCenterWorldCached() - targetCenter;
            const float d2 = d.x * d.x + d.y * d.y + d.z * d.z;
            if (d2 > md2) // No collision possible
                continue;

            // Precise check:
            ++expensiveCollisionChecks;
            if (_elements[i]->intersects(target))
                collisions.push_back(static_cast<int>(i));
        }
        return collisions;
    }

    // Single-pass collision against multiple targets.
    // Checks each bullet against targets in order and triggers at most one hit
    // callback per bullet (the first target hit).
    // If deactivateOnHit=true, deactivates bullet immediately (swap-remove)
    // using deactivateElement(i).
    template <typename OnHit>
    void collideActiveBulletsFirstHit(
        const std::vector<HitboxedDrawableEntity *> &targets,
        bool deactivateOnHit, OnHit &&onHit) {
        if (_activeCount == 0 || targets.empty())
            return;

        static thread_local std::vector<TargetInfo> infos;
        infos.clear();
        infos.reserve(targets.size());

        const float bulletR = _elements[0]->containingSphereRadius();
        for (auto *t : targets) {
            if (!t)
                continue;
            const float maxDist = bulletR + t->containingSphereRadius();
            infos.push_back(TargetInfo{.t = t,
                                       .center = t->hitboxCenterWorld(),
                                       .md2 = maxDist * maxDist});
        }

        for (size_t i = 0; i < _activeCount;) {
            const glm::vec3 bc = _elements[i]->hitboxCenterWorldCached();
            bool hit = false;

            for (size_t ti = 0; ti < infos.size(); ++ti) {
                ++cheapCollisionChecks;
                const glm::vec3 d = bc - infos[ti].center;
                const float d2 = d.x * d.x + d.y * d.y + d.z * d.z;
                if (d2 > infos[ti].md2)
                    continue;

                ++expensiveCollisionChecks;
                if (_elements[i]->intersects(infos[ti].t)) {
                    onHit(i, ti, infos[ti].t);
                    hit = true;
                    if (deactivateOnHit) {
                        deactivateElement(i);
                        // bullet swapped into i still needs processing.
                        break;
                    }
                    // Not deactivating: keep bullet and continue to next.
                    break;
                }
            }

            if (hit && deactivateOnHit) {
                continue;
            }
            ++i;
        }
    }

    void setupInstancedDrawing(unsigned int bulletMeshVAO,
                               unsigned int _meshIndecesCount) {
        instanceTransforms = new InstanceAffine[MAX_BULLETS];
        glGenBuffers(1, &instancedVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);

        // FIX 1: Changed GL_STATIC_DRAW to GL_DYNAMIC_DRAW
        // FIX 2: Passed nullptr instead of garbage array pointer to safely
        // reserve space
        glBufferData(GL_ARRAY_BUFFER, MAX_BULLETS * sizeof(InstanceAffine),
                     nullptr, GL_DYNAMIC_DRAW);

        meshVAO = bulletMeshVAO;
        meshIndecesCount = _meshIndecesCount;

        glBindVertexArray(meshVAO);

        const GLsizei stride = static_cast<GLsizei>(sizeof(InstanceAffine));
        const std::size_t vec3Size = sizeof(glm::vec3);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride,
                              (void *)(1 * vec3Size));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, stride,
                              (void *)(2 * vec3Size));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, stride,
                              (void *)(3 * vec3Size));

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

        // 1. Gather the latest transform matrices on the CPU (compact affine).
        for (size_t i = 0; i < count; i++) {
            const glm::mat4 &local =
                _elements[i]->localWithShapeTransformCached();
            const glm::vec3 p = _elements[i]->get_pos();
            instanceTransforms[i].col0 = glm::vec3(local[0]);
            instanceTransforms[i].col1 = glm::vec3(local[1]);
            instanceTransforms[i].col2 = glm::vec3(local[2]);
            instanceTransforms[i].col3 = glm::vec3(local[3]) + p;
        }

        // 2. Bind the VBO and upload the fresh data to the GPU
        // (You must bind the VBO here to tell OpenGL WHICH buffer to copy data
        // into)
        glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0, // Offset from start of buffer
                        count * sizeof(InstanceAffine), // Size of data
                        &instanceTransforms[0]          // Pointer to CPU data
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