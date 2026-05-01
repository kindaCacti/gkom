#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <random>
#include <memory>
#include <vector>

#include "camera.h"
#include "entities/player.h"
#include "shaders/utils.h"
#include "shaders/shader_params.h"
#include "mesh/mesh_loader.h"
#include "./shaders/shader_s.h"
#include "shapes/shape_factory.h"
#include "entities/player.h"
#include "defines.h"
#include "game.h"
#include "input.h"
// settings

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow *window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    {
        Game game;
        glfwSetFramebufferSizeCallback(
            window, [](GLFWwindow *window, int width, int height) {
                Game *game =
                    reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                if (game) {
                    game->onFramebufferResize(window, width, height);
                }
            });
        glfwSetWindowUserPointer(window, &game);
        glfwSetCursorPosCallback(
            window, [](GLFWwindow *window, double xpos, double ypos) {
                Game *game =
                    reinterpret_cast<Game *>(glfwGetWindowUserPointer(window));
                if (game) {
                    game->onMouseMove(window, xpos, ypos);
                }
            });
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        game.loadAssets();
        game.setupDefaultScene();

        glEnable(GL_DEPTH_TEST);
        float lastFrameTime = static_cast<float>(glfwGetTime());
        while (!glfwWindowShouldClose(window)) {
            float currentFrameTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
            processInput(window, game, deltaTime);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            game.updateCamera();
            game.drawEntities();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}