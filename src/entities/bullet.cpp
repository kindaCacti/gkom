#include "bullet.h"

#include <cmath>
#include <glm/glm.hpp>
#include <memory>

#include "../defines.h"
#include "../shaders/shader_s.h"
#include "entity.h"

void Bullet::refreshMoveDir() {
    // Convention: bullets move along local +X rotated by Euler degrees.
    glm::vec3 dir = glm::vec3(getEulerRotationMatrix(_direction) *
                              glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    const float len2 = glm::dot(dir, dir);
    if (len2 < 1e-10f) {
        _moveDir = glm::vec3(1.0f, 0.0f, 0.0f);
        return;
    }
    _moveDir = dir / std::sqrt(len2);
}

void Bullet::refreshCachedLocal() {
    // Cache rotated hitbox offset and the local (no-translation) matrix used
    // for instanced drawing.
    const glm::mat4 R = getEulerRotationMatrix(_direction);
    const glm::vec3 off = _hitbox.centerOffset();
    const float off2 = glm::dot(off, off);
    if (off2 < 1e-12f) {
        _hitboxOffsetWorld = glm::vec3(0.0f);
    } else {
        _hitboxOffsetWorld = glm::vec3(R * glm::vec4(off, 0.0f));
    }

    const glm::mat4 S = glm::scale(glm::mat4(1.0f), _scale);
    const glm::mat4 shapeT =
        _shape ? _shape->transform.getMatrix() : glm::mat4(1.0f);
    _localWithShape = R * S * shapeT;
}

void Bullet::setDirection(glm::vec3 direction) {
    _direction = std::move(direction);
    setRotation(_direction.x, _direction.y, _direction.z);
    refreshMoveDir();
    refreshCachedLocal();
}

void Bullet::setShape(std::unique_ptr<Shape> &&shape) {
    HitboxedDrawableEntity::setShape(std::move(shape));
    refreshCachedLocal();
}

glm::mat4 Bullet::getTransformMatrixWithShapeTransform() const {
    // Fast path: translation changes each frame; everything else is cached.
    return glm::translate(glm::mat4(1.0f), _pos) * _localWithShape;
}
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"

void Bullet::rotateTowardsTarget(float delta_time, glm::vec3 target) {
    // float angle = atan2(target.z - _pos.z, target.x - _pos.x);
    // float current_angle = atan2(_direction.z, _direction.x);
    // float angle_diff = angle - current_angle;

    // // Normalize the angle difference to the range [-pi, pi]
    // while (angle_diff > glm::pi<float>()) angle_diff -= 2.0f *
    // glm::pi<float>(); while (angle_diff < -glm::pi<float>()) angle_diff
    // += 2.0f * glm::pi<float>();

    // float max_angle_change = MAX_ANGLE_CHANGE_PER_SECOND * delta_time;
    // float angle_change = std::max(-max_angle_change,
    // std::min(max_angle_change, angle_diff)); float new_angle = current_angle
    // + angle_change; _direction.x = glm::degrees(cos(new_angle)); _direction.z
    // = glm::degrees(sin(new_angle));
}

void Bullet::step(float delta_time, glm::vec3 target) {
    rotateTowardsTarget(delta_time, target);
    // Keep visual + hitbox rotation aligned with movement direction.
    setRotation(_direction.x, _direction.y, _direction.z);
    _pos += delta_time * _speed * _moveDir;
}