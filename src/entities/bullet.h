#ifndef ENTITIES_BULLET_H
#define ENTITIES_BULLET_H

#include <glm/glm.hpp>
#include <memory>

#include "../defines.h"
#include "../shaders/shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"

class Bullet : public HitboxedDrawableEntity {
    float _speed;
    glm::vec3 _direction;

  public:
    Bullet(std::unique_ptr<Shape> &&shape, float speed, glm::vec3 &direction)
        : HitboxedDrawableEntity(std::move(shape)), _speed(speed),
          _direction(direction) {}
    Bullet(const Bullet &) = default;
    Bullet(Bullet &&) = default;
    Bullet &operator=(Bullet &&bullet) = default;
    Bullet &operator=(const Bullet &bullet) = default;
    ~Bullet() = default;

    void rotateTowardsTarget(float delta_time, glm::vec3 target);
    void step(float delta_time, glm::vec3 target);
    void setSpeed(float speed) { _speed = speed; }
    void setDirection(glm::vec3 direction) {
        _direction = std::move(direction);
    }
};

#endif