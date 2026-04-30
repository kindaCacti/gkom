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

    void rotate(const float x_rot = 0.0f, const float y_rot = 0.0f,
                const float z_rot = 0.0f) {
        _rot.x += x_rot;
        _rot.y += y_rot;
        _rot.z += z_rot;
    }

    void set_rotation(const float x_rot = 0.0f, const float y_rot = 0.0f,
                      const float z_rot = 0.0f) {
        _rot.x = x_rot;
        _rot.y = y_rot;
        _rot.z = z_rot;
    }

    virtual glm::vec3 get_pos() const { return _pos; }
    virtual void set_scale(glm::vec3 &&scale) { _scale = std::move(scale); }
    virtual void set_position(glm::vec3 &&pos) { _pos = std::move(pos); }
    virtual void set_rotation(glm::vec3 &&rot) { _rot = std::move(rot); }
};

class DrawableEntity : public Entity {
  public:
    DrawableEntity() : Entity() {}

    virtual void draw(Shader &shader) const = 0;
    virtual ~DrawableEntity() = default;
};

#endif