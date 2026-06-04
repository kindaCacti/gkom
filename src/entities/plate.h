#ifndef PLATE_H
#define PLATE_H

#include <glm/glm.hpp>
#include <memory>

#include "../shaders/shader_s.h"
#include "entity.h"
#include "../mesh/mesh.h"
#include "../shaders/shader_s.h"
#include "../shapes/shape.h"
#include "../shapes/shape_factory.h"
#include "entity.h"

class Plate : public HitboxedDrawableEntity {

  public:
    Plate() = delete;
    Plate(std::unique_ptr<Shape> &&shape)
        : DrawableEntity(std::move(shape)), HitboxedDrawableEntity(nullptr) {
        setHitboxShape(Hitbox::Shape::Cylinder);
    }
};

#endif