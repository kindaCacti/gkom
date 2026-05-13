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
#include "../bullet_buffer.h"

std::shared_ptr<Bullet> emiter::shootIfTime(ShapeFactory &shape_factory,
                                            float currentTime, float speed) {
    if (currentTime - _lastShotTime < _timeBetweenShots) {
        return nullptr;
    }

    auto bulletAsset = shape_factory.createShape(BULLET_ASSET_NAME);
    bulletAsset->transform.scale(glm::vec3(0.2f));
    bulletAsset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    bulletAsset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
    return shoot(std::move(bulletAsset), currentTime, speed);
}

void emiter::shootIfTime(ShapeFactory &shapeFactory, float currentTime,
                         float speed, BulletBuffer &bulletBuffer) {
    if (currentTime - _lastShotTime < _timeBetweenShots) {
        return;
    }

    auto bulletAsset = shapeFactory.createShape(BULLET_ASSET_NAME);
    bulletAsset->transform.scale(glm::vec3(0.2f));
    bulletAsset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    bulletAsset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));

    shoot(std::move(bulletAsset), currentTime, speed, bulletBuffer);
}

std::shared_ptr<Bullet> emiter::shoot(std::unique_ptr<Shape> &&bulletShape,
                                      float currentTime, float speed) {
    _lastShotTime = currentTime;
    std::shared_ptr<Bullet> newBullet =
        std::make_shared<Bullet>(std::move(bulletShape), speed, _rot);
    newBullet->setPosition(_pos);
    newBullet->setRotation(_rot);
    newBullet->setScale(_scale);
    return newBullet;
}

void emiter::shoot(std::unique_ptr<Shape> &&bullet_shape, float current_time,
                   float speed, BulletBuffer &bulletBuffer) {
    _lastShotTime = current_time;
    auto newBullet =
        bulletBuffer.createBullet(std::move(bullet_shape), speed, _rot);
    newBullet->setPosition(_pos);
    newBullet->setRotation(_rot);
    newBullet->setScale(_scale);
}