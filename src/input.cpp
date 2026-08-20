#include "input.h"

#include <cmath>

#include "defines.h"
#include "game.h"
#include"globals.h"

#include "ui/ui.h"

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

    auto moveWithCollision = [&](const glm::vec3 &move_vec) {
        struct CollisionScore {
            int collisions = 0;
            float sumDist2 = 0.0f;
        };

        auto collidesAnyPlate = [&]() {
            for (const auto &plate : game.plates) {
                if (game.player->fastCollisionCheck(plate.get())) {
                    return true;
                }
            }
            return false;
        };

        auto collisionScore = [&]() -> CollisionScore {
            CollisionScore s;
            const glm::vec3 pc = game.player->hitboxCenterWorld();
            for (const auto &plate : game.plates) {
                if (game.player->fastCollisionCheck(plate.get())) {
                    ++s.collisions;
                    const glm::vec3 qc = plate->hitboxCenterWorld();
                    const glm::vec3 d = qc - pc;
                    s.sumDist2 += d.x * d.x + d.y * d.y + d.z * d.z;
                }
            }
            return s;
        };

        auto isBetter = [](const CollisionScore &a, const CollisionScore &b) {
            if (a.collisions != b.collisions)
                return a.collisions < b.collisions;
            // If collisions count is the same, prefer being farther away from
            // colliding hitbox centers (heuristic for moving "out").
            return a.sumDist2 > b.sumDist2 + 1e-6f;
        };

        // Sub-step a little to avoid tunneling on occasional deltaTime spikes,
        // but cap the number of steps to avoid CPU spikes.
        const float maxStepLen = 0.35f;
        const float len = glm::length(move_vec);
        const int stepsRaw =
            std::max(1, static_cast<int>(std::ceil(len / maxStepLen)));
        const int steps = std::min(stepsRaw, 4);
        const glm::vec3 perStep = move_vec / float(steps);

        bool startedIntersecting = collidesAnyPlate();
        CollisionScore currentScore;
        if (startedIntersecting) {
            currentScore = collisionScore();
        }

        auto tryMove = [&](const glm::vec3 &delta) {
            if (delta.x == 0.0f && delta.y == 0.0f && delta.z == 0.0f)
                return true;

            const glm::vec3 beforePos = game.player->get_pos();

            game.player->move(delta.x, delta.y, delta.z);

            // Common case: we didn't start inside an obstacle.
            // Only a boolean collision check is needed (much cheaper than a
            // full score).
            if (!startedIntersecting) {
                if (collidesAnyPlate()) {
                    game.player->setPosition(beforePos.x, beforePos.y,
                                             beforePos.z);
                    return false;
                }
                return true;
            }

            // Special case: we started intersecting; use score to allow moving
            // out.
            const CollisionScore afterScore = collisionScore();
            if (afterScore.collisions == 0) {
                // Escaped collision: switch to normal mode for remaining steps.
                startedIntersecting = false;
                return true;
            }

            if (isBetter(afterScore, currentScore)) {
                currentScore = afterScore;
                return true;
            }

            game.player->setPosition(beforePos.x, beforePos.y, beforePos.z);
            return false;
        };

        for (int s = 0; s < steps; ++s) {
            if (tryMove(perStep))
                continue;
            // Slide along obstacles.
            tryMove(glm::vec3(perStep.x, 0.0f, 0.0f));
            tryMove(glm::vec3(0.0f, perStep.y, 0.0f));
            tryMove(glm::vec3(0.0f, 0.0f, perStep.z));
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
        game.movePlate(deltaTime);
    } else {
        game.stopMovingPlate();
    }
    if (isPressed(window, KEYBIND_RESET)) {
        game.restartGame();
    }
    if (isPressed(window, KEYBIND_SHOW_HITBOXES) and !gameStateData.wasPressedBefore(KEYBIND_SHOW_HITBOXES)) {
        game.settings.showHitboxes = !game.settings.showHitboxes;
        gameStateData.holdDownKey(KEYBIND_SHOW_HITBOXES);
    } else if(!isPressed(window, KEYBIND_SHOW_HITBOXES)) {
        gameStateData.releaseKey(KEYBIND_SHOW_HITBOXES);
    }
    if (isPressed(window, KEYBIND_SHOW_SPAWNING_AREAS)) {
        game.settings.showSpawningAreas = !game.settings.showSpawningAreas;
    }
}
