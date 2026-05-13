#include <glm/glm.hpp>
#include <memory>

#include "../shaders/shader_s.h"
#include "entity.h"
#include "bullet.h"
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"
#include "../defines.h"
#include "emiter.h"

std::shared_ptr<Bullet> emiter::shootIfTime(ShapeFactory &shape_factory,
                                            float current_time, float speed) {
    if (current_time - _last_shot_time < _time_between_shots) {
        return nullptr;
    }

    auto bullet_asset = shape_factory.createShape(BULLET_ASSET_NAME);
    bullet_asset->transform.scale(glm::vec3(0.2f));
    bullet_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    bullet_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
    return shoot(std::move(bullet_asset), current_time, speed);
}

std::shared_ptr<Bullet> emiter::shoot(std::unique_ptr<Shape> &&bullet_shape,
                                      float current_time, float speed) {
    _last_shot_time = current_time;
    std::shared_ptr<Bullet> new_bullet =
        std::make_shared<Bullet>(std::move(bullet_shape), speed, _rot);
    new_bullet->setPosition(_pos);
    new_bullet->setRotation(_rot);
    new_bullet->setScale(_scale);
    return new_bullet;
}