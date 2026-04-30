#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <memory>

#include "../shaders/shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"

class Player : public DrawableEntity {
    std::unique_ptr<Shape> _shape;

    void _set_shape_translation() const;

  public:
    Player(std::unique_ptr<Shape> &&shape)
        : DrawableEntity(), _shape(std::move(shape)) {}

    void draw(Shader &shader) const override;
    void update(float deltaTime);
};

#endif