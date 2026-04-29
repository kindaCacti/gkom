#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>

class Entity {
  protected:
    glm::vec3 _pos;
    glm::mat4 _transform;

  public:
    Entity() : _pos(0.f, 0.f, 0.f), _transform(1.0f) {}

    virtual void move(const float dx = 0.f, const float dy = 0.f,
                      const float dz = 0.f) {
        _pos.x += dx;
        _pos.y += dy;
        _pos.z += dz;
    }

    virtual void move_to(const float x = 0.f, const float y = 0.f,
                         const float z = 0.f) {
        _pos.x += x;
        _pos.y += y;
        _pos.z += z;
    }
};

class DrawableEntity {
  public:
    virtual void draw(Shader &shader) const = 0;

    virtual ~DrawableEntity() = default;
};

#endif