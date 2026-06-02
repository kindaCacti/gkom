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

  public:
    EnemyType type;

    Enemy() = default;
    Enemy(std::unique_ptr<Shape> &&shape, EnemyType type, float currentTime)
        : DrawableEntity(std::move(shape)), HitboxedDrawableEntity(nullptr),
          emiter(nullptr, currentTime, 10.0f), type(type) {
        updateTimeBetweenShots();
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

    void updateTimeBetweenShots() {
        switch (type) {
        case ORDINARY_COFFEE:
            _timeBetweenShots = 3.0f;
            break;
        case ESPRESSO:
            _timeBetweenShots = 1.0f;
            break;
        case COFFEE2GO:
            _timeBetweenShots = 2.0f;
            break;
        case COFFEE_MAKER:
            _timeBetweenShots = 2.0f;
            break;
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }

    void upgrade(ShapeFactory &shapeFactory) {
        switch (type) {
        case ORDINARY_COFFEE:
            type = ESPRESSO;
            _shape = shapeFactory.createShape(Enemy::getAssetName(ESPRESSO));
            updateHitboxFromShape();
            updateTimeBetweenShots();
            break;
        case ESPRESSO:
            type = COFFEE2GO;
            _shape = shapeFactory.createShape(Enemy::getAssetName(COFFEE2GO));
            updateHitboxFromShape();
            updateTimeBetweenShots();
            break;
        case COFFEE2GO:
            type = COFFEE_MAKER;
            _shape =
                shapeFactory.createShape(Enemy::getAssetName(COFFEE_MAKER));
            updateHitboxFromShape();
            updateTimeBetweenShots();
            break;
        case COFFEE_MAKER:
            _timeBetweenShots = std::max(0.5f, _timeBetweenShots - 0.2f);
            break;
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }
};

#endif