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
#include "../bullet_buffer.h"

class emiter : public virtual Entity {
  protected:
    float _lastShotTime;
    float _timeBetweenShots;

  public:
    void setTimeBetweenShots(float timeBetweenShots) {
        _timeBetweenShots = timeBetweenShots;
    }

    virtual ~emiter() = default;

    float getTimeBetweenShots() const { return _timeBetweenShots; }

    emiter(float currentTime, float timeBetweenShots)
        : Entity(), _lastShotTime(currentTime),
          _timeBetweenShots(timeBetweenShots) {}

    virtual std::vector<std::shared_ptr<Bullet>>
    shootIfTime(ShapeFactory &shapeFactory, float currentTime, float speed);

    virtual void shootIfTime(ShapeFactory &shapeFactory, float currentTime,
                             float speed, BulletBuffer &bulletBuffer);

    virtual std::vector<std::shared_ptr<Bullet>>
    shoot(std::unique_ptr<Shape> &&bulletShape, float currentTime, float speed);

    virtual void shoot(std::unique_ptr<Shape> &&bullet_shape,
                       float current_time, float speed,
                       BulletBuffer &bulletBuffer);
};

#endif