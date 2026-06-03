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

    auto moveWithCollision = [&](const glm::vec3 &move_vec) {
        game.player->move(move_vec.x, move_vec.y, move_vec.z);
        for (const auto &plate : game.plates) {
            if (game.player->check_3D_collision(plate.get())) {
                game.player->move(-move_vec.x, -move_vec.y, -move_vec.z);
                if (game.player->check_3D_collision(plate.get())) {
                    // if still colliding, allow movement to avoid getting stuck
                    game.player->move(move_vec.x, move_vec.y, move_vec.z);
                }
                break;
            }
        }
    };

    if (isPressed(window, KEYBIND_MOVE_FORWARD)) {
        glm::vec3 moveVec = {forward_dir.x, forward_dir.y, forward_dir.z};
        moveWithCollision(moveVec);
    }
    if (isPressed(window, KEYBIND_MOVE_BACKWARD)) {
        glm::vec3 moveVec = {-forward_dir.x, -forward_dir.y, -forward_dir.z};
        moveWithCollision(moveVec);
    }
    if (isPressed(window, KEYBIND_MOVE_LEFT)) {
        glm::vec3 moveVec = {-right_dir.x, -right_dir.y, -right_dir.z};
        moveWithCollision(moveVec);
    }
    if (isPressed(window, KEYBIND_MOVE_RIGHT)) {
        glm::vec3 moveVec = {right_dir.x, right_dir.y, right_dir.z};
        moveWithCollision(moveVec);
    }

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
