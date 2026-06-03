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
#include "enemy.h"
#include "../bullet_buffer.h"

// combine bullets from primitive emitters

std::vector<std::shared_ptr<Bullet>>
Enemy::shootIfTime(ShapeFactory &shapeFactory, float currentTime, float speed) {
    std::vector<std::shared_ptr<Bullet>> bullets;
    for (auto &emitter : _primitiveEmitters) {
        auto oldPos = emitter.get_pos();
        auto oldRot = emitter.get_rot();
        emitter.setPosition(_pos + oldPos);
        emitter.setRotation(_rot + oldRot);
        auto newBullets = emitter.shootIfTime(shapeFactory, currentTime, speed);
        emitter.setPosition(oldPos);
        emitter.setRotation(oldRot);
        bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
    }
    return bullets;
}

void Enemy::shootIfTime(ShapeFactory &shapeFactory, float currentTime,
                        float speed, BulletBuffer &bulletBuffer) {
    for (auto &emitter : _primitiveEmitters) {
        auto oldPos = emitter.get_pos();
        auto oldRot = emitter.get_rot();
        emitter.setPosition(_pos + oldPos);
        emitter.setRotation(_rot + oldRot);
        emitter.shootIfTime(shapeFactory, currentTime, speed, bulletBuffer);
        emitter.setPosition(oldPos);
        emitter.setRotation(oldRot);
    }
}

std::vector<std::shared_ptr<Bullet>>
Enemy::shoot(std::unique_ptr<Shape> &&bulletShape, float currentTime,
             float speed) {
    std::vector<std::shared_ptr<Bullet>> bullets;
    for (auto &emitter : _primitiveEmitters) {
        auto oldPos = emitter.get_pos();
        auto oldRot = emitter.get_rot();
        emitter.setPosition(_pos + oldPos);
        emitter.setRotation(_rot + oldRot);
        auto newBullets =
            emitter.shoot(std::move(bulletShape), currentTime, speed);
        emitter.setPosition(oldPos);
        emitter.setRotation(oldRot);
        bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
    }
    return bullets;
}

void Enemy::shoot(std::unique_ptr<Shape> &&bulletShape, float currentTime,
                  float speed, BulletBuffer &bulletBuffer) {
    for (auto &emitter : _primitiveEmitters) {
        auto oldPos = emitter.get_pos();
        auto oldRot = emitter.get_rot();
        emitter.setPosition(_pos + oldPos);
        emitter.setRotation(_rot + oldRot);
        emitter.shoot(std::move(bulletShape), currentTime, speed, bulletBuffer);
        emitter.setPosition(oldPos);
        emitter.setRotation(oldRot);
    }
}