#include "input.h"

#include "defines.h"
#include "game.h"

#include "ui.h"

static bool isPressed(GLFWwindow *window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void processInput(GLFWwindow *window, Game &game, float deltaTime,
                  AppState &currentState) {
    if (window == nullptr)
        return;

    if (isPressed(window, KEYBIND_EXIT))
        glfwSetWindowShouldClose(window, true);

    static bool wasPausePressed = false;
    bool isPausePressed = isPressed(window, KEYBIND_PAUSE);

    if (isPausePressed && !wasPausePressed) {
        if (currentState == AppState::InGame) {
            currentState = AppState::Paused;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            //saving pause start
            game.timeBundle.pauseStartTime = static_cast<float>(glfwGetTime());

        } else if (currentState == AppState::Paused) {
            currentState = AppState::InGame;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            float currentTime = static_cast<float>(glfwGetTime());

            // changing time of playing
            game.timeBundle.gameStartTime +=
                (currentTime - game.timeBundle.pauseStartTime);

            game.timeBundle.lastFrameTime = currentTime;
        }
    }
    wasPausePressed = isPausePressed;

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

    if (glfwGetMouseButton(window, KEYBIND_MOVE_PLATE) == GLFW_PRESS) {
        game.movePlate();
    }
    if (isPressed(window, KEYBIND_RESET)) {
        game.restartGame();
    }
    if (isPressed(window, KEYBIND_SHOW_HITBOXES)) {
        game.settings.showHitboxes = !game.settings.showHitboxes;
    }
    if (isPressed(window, KEYBIND_SHOW_SPAWNING_AREAS)) {
        game.settings.showSpawningAreas = !game.settings.showSpawningAreas;
    }
}
