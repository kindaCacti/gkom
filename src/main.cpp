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

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window, Player &p);

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int tex0 = createNoiseTexture2D(512, 512);

    {
        Game game;
        game.loadAssets();
        game.setupDefaultScene();

        glEnable(GL_DEPTH_TEST);
        float lastFrameTime = static_cast<float>(glfwGetTime());
        while (!glfwWindowShouldClose(window)) {
            float currentFrameTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
            processInput(window, game.player);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex0);

            game.updateCamera();
            game.drawEntities();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glDeleteTextures(1, &tex0);
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, Player &p) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        p.move(0.f, 0.f, MOVEMENT_SPEED);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        p.move(0.f, 0.f, -MOVEMENT_SPEED);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        p.move(MOVEMENT_SPEED, 0.f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        p.move(-MOVEMENT_SPEED, 0.f);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}
