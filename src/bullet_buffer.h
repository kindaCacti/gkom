#ifndef ELEMENT_BUFFER_H
#define ELEMENT_BUFFER_H

#include <vector>
#include <glm/glm.hpp>

#include "./entities/bullet.h"
#include "defines.h"

class BulletBuffer {
    std::vector<std::shared_ptr<Bullet>> _elements;
    size_t _activeCount = 0;

  public:
    BulletBuffer() = default;
    BulletBuffer(const BulletBuffer &) = default;
    BulletBuffer(BulletBuffer &&) = default;
    BulletBuffer &operator=(const BulletBuffer &) = default;
    BulletBuffer &operator=(BulletBuffer &&) = default;
    ~BulletBuffer() = default;

    size_t activeElementCount() const { return _activeCount; }

    std::shared_ptr<Bullet> createBullet(std::unique_ptr<Shape> &&shape,
                                         float speed, glm::vec3 &direction) {
        if (_activeCount == _elements.size()) {
            _elements.push_back(
                std::make_shared<Bullet>(std::move(shape), speed, direction));
            return _elements[_activeCount++];
        }

        _elements[_activeCount]->setSpeed(speed);
        _elements[_activeCount]->setDirection(direction);
        _elements[_activeCount]->setShape(std::move(shape));

        return _elements[_activeCount++];
    }

    void deactivateElement(size_t position) {
        --_activeCount;
        std::swap(_elements[position], _elements[_activeCount]);
    }

    void moveRemoveActiveElements(float deltaTime, glm::vec3 &&target) {
        moveRemoveActiveElements(deltaTime, target);
    }

    void moveRemoveActiveElements(float deltaTime, glm::vec3 &target) {
        for (size_t i = 0; i < _activeCount; i++) {
            _elements[i]->step(deltaTime, target);
            auto bulletPosition = _elements[i]->get_pos();
            if (bulletPosition.x * bulletPosition.x +
                    bulletPosition.z * bulletPosition.z >=
                AREA_RADIUS_SQ) {
                deactivateElement(i);
            }
        }
    }

    void drawActiveElements(Shader &shader) {
        for (size_t i = 0; i < _activeCount; i++) {
            _elements[i]->draw(shader);
        }
    }

    bool
    checkActiveBulletCollision(HitboxedDrawableEntity *hitboxedDrawableEntity) {
        for (size_t i = 0; i < _activeCount; i++) {
            if (_elements[i]->intersects(hitboxedDrawableEntity))
                return true;
        }
        return false;
    }
};

#endif