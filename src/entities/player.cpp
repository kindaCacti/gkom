#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"

void Player::_set_shape_translation() const {
    _shape->reset_transform();
    _shape->translate(_pos);
    _shape->rotate(_rot);
    _shape->scale(_scale);
}

void Player::draw(Shader &shader) const {
    _set_shape_translation();
    _shape->draw(shader.ID);
}