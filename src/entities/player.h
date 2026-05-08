#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <memory>

#include "../shaders/shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"
#include "../hitbox/hitbox.h"

class Player : public HitboxedDrawableEntity {

  public:
    Player() = default;
    Player(std::unique_ptr<Shape>&& shape)
        : HitboxedDrawableEntity(std::move(shape)) {}
    // Player& operator=(Player&& player) = default;
};

#endif