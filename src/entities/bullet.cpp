#include "bullet.h"

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


void Bullet::rotateTowardsTarget(float delta_time, glm::vec3 target) {
    // float angle = atan2(target.z - _pos.z, target.x - _pos.x);
    // float current_angle = atan2(_direction.z, _direction.x);
    // float angle_diff = angle - current_angle;

    // // Normalize the angle difference to the range [-pi, pi]
    // while (angle_diff > glm::pi<float>()) angle_diff -= 2.0f * glm::pi<float>();
    // while (angle_diff < -glm::pi<float>()) angle_diff += 2.0f * glm::pi<float>();

    // float max_angle_change = MAX_ANGLE_CHANGE_PER_SECOND * delta_time;
    // float angle_change = std::max(-max_angle_change, std::min(max_angle_change, angle_diff));
    // float new_angle = current_angle + angle_change;
    // _direction.x = glm::degrees(cos(new_angle));
    // _direction.z = glm::degrees(sin(new_angle));
}

void Bullet::step(float delta_time, glm::vec3 target) {
    rotateTowardsTarget(delta_time, target);
    glm::vec3 direction = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * getEulerRotationMatrix(_direction);
    direction = glm::normalize(direction);
    _pos.x += delta_time * _speed * direction.x;
    _pos.y += delta_time * _speed * direction.y;
    _pos.z += delta_time * _speed * direction.z;
}