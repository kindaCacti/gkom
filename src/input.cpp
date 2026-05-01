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

    if (isPressed(window, KEYBIND_MOVE_FORWARD))
        game.player.move(0.f, step, 0.f);
    if (isPressed(window, KEYBIND_MOVE_BACKWARD))
        game.player.move(0.f, -step, 0.f);
    if (isPressed(window, KEYBIND_MOVE_LEFT))
        game.player.move(-step, 0.f, 0.f);
    if (isPressed(window, KEYBIND_MOVE_RIGHT))
        game.player.move(step, 0.f, 0.f);
}
