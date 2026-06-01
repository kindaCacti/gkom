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
    Enemy(std::unique_ptr<Shape> &&shape, EnemyType type, float currentTime,
          float time_between_shots)
        : DrawableEntity(std::move(shape)), HitboxedDrawableEntity(nullptr),
          emiter(nullptr, currentTime, time_between_shots), type(type) {
        switch (type) {
        case ESPRESSO:
            _shape->transform.scale(glm::vec3(0.5f));
            break;
        case COFFEE2GO:
            _shape->transform.scale(glm::vec3(0.7f));
            break;
        case COFFEE_MAKER:
            _shape->transform.scale(glm::vec3(0.9f));
            break;
        case ORDINARY_COFFEE:
            _shape->transform.scale(glm::vec3(0.6f));
            break;
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }

    static std::string getAssetName(EnemyType type) {
        switch (type) {
        case ESPRESSO:
            return "espresso";
        case COFFEE2GO:
            return "coffee2go";
        case COFFEE_MAKER:
            return "coffee_maker";
        case ORDINARY_COFFEE:
            return "ordinary_coffee";
        default:
            throw std::runtime_error("Unknown EnemyType");
        }
    }
};

#endif