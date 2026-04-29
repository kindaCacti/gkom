#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"

void Player::draw(Shader &shader) const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, _pos);
    time_t time_v;
    time(&time_v);
    float angle = 20.0f * static_cast<float>(glfwGetTime());
    model =
        glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    shader.setMat4("model", model);
    glBindVertexArray(_shape->mesh->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _shape->mesh->EBO);
    glDrawElements(GL_TRIANGLES, _shape->mesh->indexCount, GL_UNSIGNED_INT, 0);
}