#ifndef ENTITIES_ENTITY_H
#define ENTITIES_ENTITY_H

#include "../shaders/shader_s.h"
#include "../utils.h"
#include "../shapes/shape.h"

class Entity {
  protected:
    // entity ma to trzymać, bo zmiana tego zwiększa nie tylko czas potrzebny do
    // odczytu danych (co jest ważne), a także zwiększa szansę na błędy przy
    // dzieleniu przez zero z klasy transform (proszę nie zmieniać)
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

    virtual void set_position(const float x = 0.f, const float y = 0.f,
                              const float z = 0.f) {
        _pos.x += x;
        _pos.y += y;
        _pos.z += z;
    }

    virtual void rotate(const float dx_rot = 0.0f, const float dy_rot = 0.0f,
                        const float dz_rot = 0.0f) {
        _rot.x += dx_rot;
        _rot.y += dy_rot;
        _rot.z += dz_rot;
    }

    virtual void set_rotation(const float x_rot = 0.0f,
                              const float y_rot = 0.0f,
                              const float z_rot = 0.0f) {
        _rot.x = x_rot;
        _rot.y = y_rot;
        _rot.z = z_rot;
    }

    virtual void scale(const float dx = 0.f, const float dy = 0.f,
                       const float dz = 0.f) {
        _scale.x += dx;
        _scale.x += dy;
        _scale.x += dz;
    }

    virtual void set_scale(const float x = 0.f, const float y = 0.f,
                           const float z = 0.f) {
        _scale.x = x;
        _scale.x = y;
        _scale.x = z;
    }

    virtual void set_scale(glm::vec3 &&scale) { _scale = std::move(scale); }
    virtual void set_position(glm::vec3 &&pos) { _pos = std::move(pos); }
    virtual void set_rotation(glm::vec3 &&rot) { _rot = std::move(rot); }
    virtual void set_scale(glm::vec3 &scale) { _scale = glm::vec3(scale); }
    virtual void set_position(glm::vec3 &pos) { _pos = glm::vec3(pos); }
    virtual void set_rotation(glm::vec3 &rot) { _rot = glm::vec3(rot); }


    virtual glm::vec3 get_pos() const { return _pos; }
};

class DrawableEntity : public Entity {
    std::unique_ptr<Shape> _shape;

  public:
    DrawableEntity(std::unique_ptr<Shape>&& shape) : Entity(), _shape(std::move(shape)) {}
    DrawableEntity(DrawableEntity&&) = default;

    DrawableEntity& operator=(DrawableEntity&&) = default;


    virtual void draw(Shader &shader) const {
        _shape->draw(shader.ID, getTransformMatrix());
    }

    virtual glm::mat4 getTransformMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), _pos);
        glm::mat4 R = getEulerRotationMatrix(_rot);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), _scale);
        return T * R * S;
    }

    virtual ~DrawableEntity() = default;
};

#endif