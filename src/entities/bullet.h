#ifndef ENTITIES_BULLET_H
#define ENTITIES_BULLET_H

#include <glm/glm.hpp>
#include <memory>

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
    Bullet(std::unique_ptr<Shape> &&shape, float speed, glm::vec3 direction)
        : HitboxedDrawableEntity(std::move(shape)), _speed(speed), _direction(direction) {}
    Bullet& operator=(Bullet&& bullet) = default;

    void step(float delta_time) {
      glm::vec3 direction = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * getEulerRotationMatrix(_direction);
      direction = glm::normalize(direction);
      _pos.x += delta_time * _speed * direction.x;
      _pos.y += delta_time * _speed * direction.y;
      _pos.z += delta_time * _speed * direction.z;
    }
};


#endif