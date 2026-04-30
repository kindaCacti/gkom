#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <memory>

#include "../mesh/mesh.h"
#include "../shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"

class Player : public DrawableEntity, public Entity {
    std::unique_ptr<Shape> _shape;

  public:
    Player(std::unique_ptr<Shape> &&shape)
        : Entity(), _shape(std::move(shape)) {}

    void draw(Shader &shader) const override;
    void update(float deltaTime);
};

#endif