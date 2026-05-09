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

  public:
    Bullet(std::unique_ptr<Shape> &&shape, float speed)
        : HitboxedDrawableEntity(std::move(shape)), _speed(speed) {}
    Bullet& operator=(Bullet&& bullet) = default;

    void step(float delta_time) {
      _pos.x += delta_time * _speed;
    }
};


#endif