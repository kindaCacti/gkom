#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"

void Player::update(float deltaTime) {
    // For demonstration, let's just rotate the player over time
    float angle = 20.0f * deltaTime; // Rotate 20 degrees per second
    transform.rotate(angle, glm::vec3(0.f, 1.f, 0.f));
}

void Player::draw(Shader &shader) const {
    _shape->draw(shader.ID, transform.getMatrix());
}