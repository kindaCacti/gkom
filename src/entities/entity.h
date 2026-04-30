#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>

class Entity {
  protected:
    glm::vec3 _pos;   // center point of an entity (in x y and z)
    glm::vec3 _rot;   // rotation of an entity (int x y and z)
    glm::vec3 _scale; // scale of an entity (in x y and z)

  public:
    Entity()
        : _pos(0.f, 0.f, 0.f), _rot(0.f, 0.f, 0.f), _scale(1.f, 1.f, 1.f) {}

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

class DrawableEntity : public Entity {
  public:
    DrawableEntity() : Entity() {}

    virtual void draw(Shader &shader) const = 0;
    virtual ~DrawableEntity() = default;
};

#endif