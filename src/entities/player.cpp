#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./player.h"
#include "../utils.h"

glm::mat4 Player::getTransformMatrix() const {
    glm::mat4 T = glm::translate(glm::mat4(1.0f), _pos);
    glm::mat4 R = getEulerRotationMatrix(_rot);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), _scale);
    return T * R * S;
}

void Player::draw(Shader &shader) const {
    _shape->draw(shader.ID, getTransformMatrix());
}