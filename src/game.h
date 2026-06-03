// proszę bez jawnych funkcji gl w tej strukturze (ma to być abstrakcja nad
// opengl i glad)

#ifndef GAME_H
#define GAME_H

enum GameControlsMode {
    THIRD_PERSON,
    TOP_DOWN,
};

const GameControlsMode CONTROLS_MODE = THIRD_PERSON;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <list>

#include "camera.h"
#include "entities/player.h"
#include "entities/emiter.h"
#include "entities/plate.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"
#include "textures/texture_factory.h"
#include "textures/texture.h"
#include "bullet_buffer.h"
#include "text/text.h"
#include "text/text_data.h"

// Forward declarations for types stored via pointers/references.
class Enemy;
class Player;
class Plate;

struct ShaderBundle {
    std::shared_ptr<Shader> gameShader;
    std::shared_ptr<Shader> textShader;
    std::shared_ptr<Shader> instancedShader;
};

struct GameSettings {
    bool instancingOn = IS_INSTANCED;
    bool benchmarkOn = BENCHMARK_ON;
    int startingEmmitersCount = 1;
    float addEmmiterAfterTime = 1.0f;
    bool showHitboxes = false;
    bool showSpawningAreas = false;
};

struct TimeBundle {
    float currentFrameTime;
    float lastFrameTime;
    float gameStartTime;

    TimeBundle()
        : currentFrameTime(static_cast<float>(glfwGetTime())),
          lastFrameTime(static_cast<float>(glfwGetTime())),
          gameStartTime(0.0f) {}

    void update() {
        lastFrameTime = currentFrameTime;
        currentFrameTime = static_cast<float>(glfwGetTime());
    }

    void reset() {
        currentFrameTime = static_cast<float>(glfwGetTime());
        lastFrameTime = currentFrameTime;
        gameStartTime = currentFrameTime;
    }
};

struct SpawnArea {
    float xMin, xMax, yMin, yMax, zLevel;

    float surfaceArea() const { return (xMax - xMin) * (yMax - yMin); }
    glm::vec3 randomSample() const {
        float x = xMin + static_cast<float>(rand()) / RAND_MAX * (xMax - xMin);
        float y = yMin + static_cast<float>(rand()) / RAND_MAX * (yMax - yMin);
        return glm::vec3(x, y, zLevel);
    }
};

struct SpawnAreas {
    std::vector<SpawnArea> areas;
    float totalSurfaceArea = 0.f;

    SpawnAreas(std::initializer_list<SpawnArea> initAreas) : areas(initAreas) {
        for (const auto &area : areas) {
            totalSurfaceArea += area.surfaceArea();
        }
    }

    glm::vec3 randomSample() const {
        float r = static_cast<float>(rand()) / RAND_MAX * totalSurfaceArea;
        float cumulative = 0.f;
        for (const auto &area : areas) {
            cumulative += area.surfaceArea();
            if (r <= cumulative) {
                return area.randomSample();
            }
        }
        return areas.back().randomSample(); // Fallback, should rarely happen
    }

    void drawDebug(const std::shared_ptr<Shader> &shader, Shape &shape) const {
        for (const auto &area : areas) {
            shape.transform = Transform();
            shape.transform.translate(glm::vec3((area.xMin + area.xMax) / 2.f,
                                                (area.yMin + area.yMax) / 2.f,
                                                area.zLevel + 0.5f));
            shape.transform.scale(glm::vec3((area.xMax - area.xMin),
                                            (area.yMax - area.yMin), 1.f));
            shape.draw(*shader, glm::mat4(1.0f), GL_LINES);
        }
    }
};

const SpawnAreas SPAWNING_AREAS({
    {-13.f, -6.f, 35.f, 39.f, 0.68f},
    {14.f, 19.f, 35.f, 39.f, 0.68f},
    {21.5f, 22.5f, 30.f, 36.f, 0.68f},
    {21.5f, 26.f, 10.f, 16.f, 0.68f},
    // Mirror on the x and y axes
    {6.f, 13.f, -39.f, -35.f, 0.68f},
    {-19.f, -14.f, -39.f, -35.f, 0.68f},
    {-22.5f, -21.5f, -36.f, -30.f, 0.68f},
    {-26.f, -21.5f, -16.f, -10.f, 0.68f},
});

struct Game {
    Camera cam;
    std::shared_ptr<Player> player;
    std::vector<std::shared_ptr<Plate>> plates;
    std::list<std::shared_ptr<Enemy>> enemies;
    std::vector<std::shared_ptr<Shape>> shapes;
    std::unique_ptr<Shape> hitboxShape;
    std::unique_ptr<Shape> spawningAreaShape;
    BulletBuffer bulletBuffer;
    ShapeFactory shapeFactory;
    TextureFactory textureFactory;
    ShaderBundle shaders;
    std::unique_ptr<Shape> axes[6]; // for debugging
    TimeBundle timeBundle;
    TextRenderer Text;
    GameSettings settings;

    Game() { loadAssets(); }

    float deltaTime() {
        return timeBundle.currentFrameTime - timeBundle.lastFrameTime;
    }
    float gameplayTime() {
        return timeBundle.currentFrameTime - timeBundle.gameStartTime;
    }
    float gameStartTime() { return timeBundle.gameStartTime; }
    float currentFrameTime() { return timeBundle.currentFrameTime; }
    float lastFrameTime() { return timeBundle.lastFrameTime; }
    void updateTimes() { timeBundle.update(); }
    void restartTimes() { timeBundle.reset(); }
    void clearNonPlayerEntities();
    void startGame();
    void restartGame();
    void setupGame();
    int loadFont();
    void updateScene();
    void drawScene();
    void doFramePreprocessing();
    void loadShaders();
    void registerMeshAsset(std::string &&name, std::string &&path,
                           Transform &&transform = Transform(),
                           std::optional<glm::vec3> color = std::nullopt);
    void loadAssets();
    void spawnPlayer();
    void resetPlayer();
    void spawnPlates();
    void resetPlates();
    void movePlate();
    void spawnEnemy(glm::vec3 position = glm::vec3(0.f),
                    glm::vec3 rotation = glm::vec3(0.f));
    void spawnRandomEnemy();
    void snapPlayerIntoArea();
    void shootIfTime(float speed);
    void moveRemoveBullets();
    void setupLights();
    void setupAxes();
    void setupTable();
    void setupScene();
    void setupDefaultScene();
    void setupBenchmarkScene();
    void updateCamera();
    void checkPlayerCollision();
    void checkPlateCollision();
    void drawEntities();
    void printStats();
    void drawText(TextData &text);
    void bundledDrawText(std::vector<TextData> &texts);
    void drawBulletsInstanced();
    void showHeart(glm::vec3 pos);
    void showPlayerLives();
    bool shouldEnd() { return player->lives <= 0; }
    void showEndScreen();

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