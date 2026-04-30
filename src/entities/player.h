#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <memory>

#include "../shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"

class Player : public DrawableEntity {
    std::unique_ptr<Shape> _shape;

  public:
    Player(std::unique_ptr<Shape> &&shape)
        : DrawableEntity(), _shape(std::move(shape)) {}

    void draw(Shader &shader) const override;
};

#endif