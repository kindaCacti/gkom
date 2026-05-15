#include "input.h"

#include "defines.h"
#include "game.h"

static bool isPressed(GLFWwindow *window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void processInput(GLFWwindow *window, Game &game, float deltaTime) {
    if (window == nullptr)
        return;

    if (isPressed(window, KEYBIND_EXIT))
        glfwSetWindowShouldClose(window, true);

    const float step = MOVEMENT_SPEED * deltaTime;
    glm::vec3 forward_dir;
    glm::vec3 right_dir;
    bool followCamera = CONTROLS_MODE == THIRD_PERSON;
    if (followCamera) {
        forward_dir = game.cam.getXYDirection();
        right_dir = glm::normalize(glm::cross(forward_dir, Z_UP));
    } else {
        forward_dir = glm::vec3(-1.f, 0.f, 0.f);
        right_dir = glm::vec3(0.f, 1.f, 0.f);
    }
    forward_dir *= step;
    right_dir *= step;

    if (isPressed(window, KEYBIND_MOVE_FORWARD))
        game.player->move(forward_dir.x, forward_dir.y, forward_dir.z);
    if (isPressed(window, KEYBIND_MOVE_BACKWARD))
        game.player->move(-forward_dir.x, -forward_dir.y, -forward_dir.z);
    if (isPressed(window, KEYBIND_MOVE_LEFT))
        game.player->move(-right_dir.x, -right_dir.y, -right_dir.z);
    if (isPressed(window, KEYBIND_MOVE_RIGHT))
        game.player->move(right_dir.x, right_dir.y, right_dir.z);
}
