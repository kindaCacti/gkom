#ifndef ENTITY_H
#define ENTITY_H

#include "../shader_s.h"
#include "../utils.h"

class Entity {
  protected:
  public:
    Transform transform;
    Entity() : transform() {}

    // virtual void move(const float dx = 0.f, const float dy = 0.f,
    //                   const float dz = 0.f) {
    //     _pos.x += dx;
    //     _pos.y += dy;
    //     _pos.z += dz;
    // }

    // virtual void move_to(const float x = 0.f, const float y = 0.f,
    //                      const float z = 0.f) {
    //     _pos.x += x;
    //     _pos.y += y;
    //     _pos.z += z;
    // }
};

class DrawableEntity {
  public:
    virtual void draw(Shader &shader) const = 0;

    virtual ~DrawableEntity() = default;
};

#endif