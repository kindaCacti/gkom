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

    std::shared_ptr<Bullet> shootIfTime(ShapeFactory& shape_factory, float current_time, float speed);
    std::shared_ptr<Bullet> shoot(std::unique_ptr<Shape>&& bullet_shape, float current_time, float speed);
};


#endif