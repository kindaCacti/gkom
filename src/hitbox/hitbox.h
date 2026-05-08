#ifndef HITBOX_H
#define HITBOX_H

#include <glm/glm.hpp>

struct Hitbox {
    glm::vec3 _hitbox_pos;
    glm::vec3 _hitbox_size;

    Hitbox(glm::vec3&& pos, glm::vec3&& size): _hitbox_pos(std::move(pos)), _hitbox_size(std::move(size)) {}
    Hitbox(glm::vec3& pos, glm::vec3& size): _hitbox_pos(pos), _hitbox_size(size) {}
    Hitbox(Hitbox&& square) = default;

    Hitbox& operator=(Hitbox&& square_hitbox) = default;

    int bottom_x() {
        return _hitbox_pos.x - _hitbox_size.x;
    }

    int bottom_y() {
        return _hitbox_pos.y - _hitbox_size.y;
    }

    int bottom_z() {
        return _hitbox_pos.z - _hitbox_size.z;
    }

    int top_x(){
        return _hitbox_pos.x + _hitbox_size.x;
    }

    int top_y() {
        return _hitbox_pos.y + _hitbox_size.y;
    }

    int top_z() {
        return _hitbox_pos.z + _hitbox_size.z;
    }

    bool x_intersects(Hitbox* other) {
        return bottom_x() < other->top_x() and top_x() > other->bottom_x();
    }

    bool y_intersects(Hitbox* other) {
        return bottom_y() < other->top_y() and top_y() > other->bottom_y();
    }

    bool z_intersects(Hitbox* other) {
        return bottom_z() < other->top_z() and top_z() > other->bottom_z();
    }

    bool instersects(Hitbox* other) {
        return x_intersects(other) and y_intersects(other) and z_intersects(other);
    }

    virtual void updateHitbox() = 0;
};

#endif