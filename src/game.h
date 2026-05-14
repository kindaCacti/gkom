// proszę bez jawnych funkcji gl w tej strukturze (ma to być abstrakcja nad
// opengl i glad)

#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <list>

#include "camera.h"
#include "./entities/player.h"
#include "./entities/emiter.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"
#include "textures/texture_factory.h"
#include "textures/texture.h"
#include "./bullet_buffer.h"

struct Game {
    Camera cam;
    std::shared_ptr<Player> player;
    std::list<std::shared_ptr<emiter>> emiters;
    std::vector<std::shared_ptr<Shape>> shapes;
    BulletBuffer bulletBuffer;
    ShapeFactory shapeFactory;
    TextureFactory textureFactory;
    std::shared_ptr<Shader> shader;
    std::unique_ptr<Shape> axes[3]; // for debugging
    float deltaTime = 0.f;
    float currentFrameTime = static_cast<float>(glfwGetTime());
    float lastFrameTime = static_cast<float>(glfwGetTime());

    Game() { loadAssets(); }

    void updateScene();
    void drawScene();
    void doFramePreprocessing();
    void loadShaders();
    void loadAssets();
    void spawnPlayer();
    void spawnEmiter(float time_between_shots,
                     glm::vec3 position = glm::vec3(0.f),
                     glm::vec3 rotation = glm::vec3(0.f));
    void spawnRandomemiter();
    void snapPlayerIntoArea();
    void shootIfTime(float speed);
    void moveRemoveBullets();
    void setupDefaultScene();
    void updateCamera();
    void checkPlayerCollision();
    void drawEntities();
    void printStats();

    void onFramebufferResize(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        cam.setAspectRatio(static_cast<float>(width) /
                           static_cast<float>(height));
    }

    void onMouseMove(GLFWwindow *window, double xpos, double ypos) {
        cam.onMouseMove(xpos, ypos);
    }

    ~Game() = default;
};

#endif