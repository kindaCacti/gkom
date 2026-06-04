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
    glm::vec3 _moveDir = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 _hitboxOffsetWorld = glm::vec3(0.0f);
    glm::mat4 _localWithShape = glm::mat4(1.0f); // R * S * shapeTransform

    void refreshMoveDir();
    void refreshCachedLocal();

  public:
    Bullet(std::unique_ptr<Shape> &&shape, float speed, glm::vec3 &direction)
        : DrawableEntity(std::move(shape)), HitboxedDrawableEntity(nullptr),
          _speed(speed), _direction(direction) {
        setHitboxShape(Hitbox::Shape::Cylinder);
        setCylinderAxis(Hitbox::CylinderAxis::X);
        setDirection(direction);
    }
    Bullet(const Bullet &) = delete;
    Bullet(Bullet &&) = default;
    Bullet &operator=(Bullet &&bullet) = default;
    Bullet &operator=(const Bullet &bullet) = delete;
    ~Bullet() = default;

    void rotateTowardsTarget(float delta_time, glm::vec3 target);
    void step(float delta_time, glm::vec3 target);
    void setSpeed(float speed) { _speed = speed; }
    void setDirection(glm::vec3 direction);

    void setShape(std::unique_ptr<Shape> &&shape) override;

    inline glm::vec3 hitboxCenterWorldCached() const {
        return _pos + _hitboxOffsetWorld;
    }

    inline const glm::mat4 &localWithShapeTransformCached() const {
        return _localWithShape;
    }

    glm::mat4 getTransformMatrixWithShapeTransform() const override;
};

#endif