#ifndef ENTITIES_EMITER_H
#define ENTITIES_EMITER_H

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


class Emmiter : public DrawableEntity {
    float _last_shot_time;
    float _time_between_shots;

  public:
    Emmiter(std::unique_ptr<Shape> &&shape, float current_time, float time_between_shots)
        : DrawableEntity(std::move(shape)), _last_shot_time(current_time), _time_between_shots(time_between_shots) {}

    std::shared_ptr<Bullet> shoot_if_time(ShapeFactory& shape_factory, float current_time, float speed) {
        if(current_time - _last_shot_time < _time_between_shots) {
            return nullptr;
        }

        auto bullet_asset = shape_factory.createShape(BULLET_ASSET_NAME);
        bullet_asset->transform.scale(glm::vec3(0.2f));
        bullet_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
        bullet_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
        return shoot(std::move(bullet_asset), current_time, speed);
    }

    std::shared_ptr<Bullet> shoot(std::unique_ptr<Shape>&& bullet_shape, float current_time, float speed) {
        _last_shot_time = current_time;
        std::shared_ptr<Bullet> new_bullet = std::make_shared<Bullet>(std::move(bullet_shape), speed, _rot);
        new_bullet->set_position(_pos);
        new_bullet->set_rotation(_rot);
        new_bullet->set_scale(_scale);
        return new_bullet;
    }
};


#endif