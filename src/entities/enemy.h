#ifndef ENEMY_H
#define ENEMY_H

#include <glm/glm.hpp>
#include <memory>

#include "../shaders/shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"
#include "emiter.h"

enum EnemyType {
    ESPRESSO,
    COFFEE2GO,
    COFFEE_MAKER,
    ORDINARY_COFFEE,
};

class Enemy : public HitboxedDrawableEntity, public emiter {
    std::vector<emiter> _primitiveEmitters;

  public:
    EnemyType type;

    Enemy() = delete;
    Enemy(std::unique_ptr<Shape> &&shape, EnemyType type, float currentTime)
        : DrawableEntity(std::move(shape)), HitboxedDrawableEntity(nullptr),
          emiter(currentTime, 2137.0f), type(type) {
        updateConfig(currentTime);
    }

    static std::string getAssetName(EnemyType type) {
        switch (type) {
        case ORDINARY_COFFEE:
            return "ordinary_coffee";
        case ESPRESSO:
            return "espresso";
        case COFFEE2GO:
            return "coffee2go";
        case COFFEE_MAKER:
            return "coffee_maker";
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }

    void setType(EnemyType newType, ShapeFactory &shapeFactory,
                 float currentTime) {
        type = newType;
        setShape(shapeFactory.createShape(getAssetName(type)));
        updateConfig(currentTime);
    }

    void updateConfig(float currentTime) {
        switch (type) {
        case ORDINARY_COFFEE: {
            _primitiveEmitters.clear();
            auto em = emiter(currentTime, 3.0f);
            em.setPosition(glm::vec3(0.f, 0.f, 0.5f));
            em.setRotation(glm::vec3(0.f, 0.f, 0.f));
            _primitiveEmitters.emplace_back(std::move(em));
        } break;
        case ESPRESSO: {
            _primitiveEmitters.clear();
            auto em = emiter(currentTime, 1.5f);
            em.setPosition(glm::vec3(0.f, 0.f, 0.3f));
            em.setRotation(glm::vec3(0.f, 0.f, 0.f));
            _primitiveEmitters.emplace_back(std::move(em));
        } break;
        case COFFEE2GO: {
            _primitiveEmitters.clear();
            auto em1 = emiter(currentTime, 3.0f);
            em1.setPosition(glm::vec3(0.f, 0.f, 0.6f));
            em1.setRotation(glm::vec3(0.f, 0.f, 0.f));
            auto em2 = emiter(currentTime + 0.25f, 3.0f);
            em2.setPosition(glm::vec3(0.f, 0.f, 1.2f));
            em2.setRotation(glm::vec3(0.f, 0.f, 0.f));
            auto em3 = emiter(currentTime + 0.5f, 3.0f);
            em3.setPosition(glm::vec3(0.f, 0.f, 1.8f));
            em3.setRotation(glm::vec3(0.f, 0.f, 0.f));
            _primitiveEmitters.emplace_back(std::move(em1));
            _primitiveEmitters.emplace_back(std::move(em2));
            _primitiveEmitters.emplace_back(std::move(em3));
        } break;
        case COFFEE_MAKER: {
            _primitiveEmitters.clear();
            auto em1 = emiter(currentTime, 2.0f);
            em1.setPosition(glm::vec3(0.f, 0.f, 0.8f));
            em1.setRotation(glm::vec3(0.f, 0.f, -5.f));
            auto em2 = emiter(currentTime + 0.25f, 2.0f);
            em2.setPosition(glm::vec3(0.f, 0.f, 0.8f));
            em2.setRotation(glm::vec3(0.f, 0.f, 0.f));
            auto em3 = emiter(currentTime + 0.5f, 2.0f);
            em3.setPosition(glm::vec3(0.f, 0.f, 0.8f));
            em3.setRotation(glm::vec3(0.f, 0.f, 5.f));
            _primitiveEmitters.emplace_back(std::move(em1));
            _primitiveEmitters.emplace_back(std::move(em2));
            _primitiveEmitters.emplace_back(std::move(em3));
        } break;
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }

    void upgrade(ShapeFactory &shapeFactory, float currentTime,
                 bool benchmarkUpgrade = false) {
        if (benchmarkUpgrade) {
            setType(COFFEE_MAKER, shapeFactory, currentTime);
            for (auto &em : _primitiveEmitters) {
                em.setTimeBetweenShots(0.05f);
            }
            return;
        }
        switch (type) {
        case ORDINARY_COFFEE:
            setType(ESPRESSO, shapeFactory, currentTime);
            break;
        case ESPRESSO:
            setType(COFFEE2GO, shapeFactory, currentTime);
            break;
        case COFFEE2GO:
            setType(COFFEE_MAKER, shapeFactory, currentTime);
            break;
        case COFFEE_MAKER:
            for (auto &em : _primitiveEmitters) {
                em.setTimeBetweenShots(
                    std::max(0.5f, em.getTimeBetweenShots() - 0.5f));
            }
            break;
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }

    virtual std::vector<std::shared_ptr<Bullet>>
    shootIfTime(ShapeFactory &shapeFactory, float currentTime,
                float speed) override;
    virtual void shootIfTime(ShapeFactory &shapeFactory, float currentTime,
                             float speed, BulletBuffer &bulletBuffer) override;
    virtual std::vector<std::shared_ptr<Bullet>>
    shoot(std::unique_ptr<Shape> &&bulletShape, float currentTime,
          float speed) override;
    virtual void shoot(std::unique_ptr<Shape> &&bullet_shape,
                       float current_time, float speed,
                       BulletBuffer &bulletBuffer) override;
};

#endif