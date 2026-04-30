#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"

void Player::draw(Shader &shader) const {
    time_t time_v;
    time(&time_v);
    float angle = 20.0f * static_cast<float>(glfwGetTime());
    _shape->setRotation(angle, glm::vec3(0.0f, 1.0f, 0.0f));
    _shape->draw(shader.ID);
}