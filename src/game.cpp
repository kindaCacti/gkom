#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <list>

#include "game.h"
#include "camera.h"
#include "./entities/player.h"
#include "./entities/emiter.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"
#include "textures/texture_factory.h"
#include "textures/texture.h"
#include "text/text.h"
#include "globals.h"

void Game::setupGame() {}

int Game::loadFont() {
    Text = TextRenderer();
    if (!Text.Init("../assets/fonts/AovelSansRounded.ttf", 48, 800, 600,
                   shaders.textShader->ID)) {
        std::cerr << "Failed while loading font" << std::endl;
        return -1; // Initialization failed
    }

    return 0;
}

void Game::updateScene() {
    snapPlayerIntoArea();
    // removeOutOfBoundsBullets();
    while (currentFrameTime / emiters.size() >
           (settings.benchmarkOn ? BENCHMARK_SPAWNING_NEW_EMMITERS_AFTER_TIME
                                 : SPAWNING_NEW_EMMITERS_AFTER_TIME)) {
        spawnRandomemiter();
    }
    shootIfTime(BULLET_SPEED);
    moveRemoveBullets();
    checkPlayerCollision();
    updateCamera();
}

void Game::drawScene() { drawEntities(); }

void Game::doFramePreprocessing() {
    lastFrameTime = currentFrameTime;
    currentFrameTime = static_cast<float>(glfwGetTime());
    deltaTime = currentFrameTime - lastFrameTime;
    deltaTime = std::min(deltaTime, 0.1f);
}

void Game::loadShaders() {
    shaders.gameShader = std::make_shared<Shader>("../shaders/blinn-phong.vs",
                                                  "../shaders/blinn-phong.fs");
    shaders.textShader =
        std::make_shared<Shader>("../shaders/text.vs", "../shaders/text.fs");
    shaders.instancedShader = std::make_shared<Shader>(
        "../shaders/instanced.vs", "../shaders/instanced.fs");
}

void Game::loadAssets() {
    textureFactory.registerTexture(
        std::make_shared<Texture>(Texture::newNoise2D(512, 512)), "noise");
    shapeFactory.registerMesh("../assets/teapot.obj", "teapot",
                              glm::vec3(0.8f, 0.5f, 0.2f));

    shapeFactory.registerMesh("../assets/table.obj", "table",
                              glm::vec3(0.8f, 0.5f, 0.2f));
    textureFactory.registerTexture(
        std::make_shared<Texture>(Texture::fromFile("../assets/wood.jpg")),
        "wood");

    shapeFactory.registerCube();

    bulletBuffer.setupInstancedDrawing(
        shapeFactory.createShape(BULLET_ASSET_NAME)->mesh.lock()->VAO,
        shapeFactory.createShape(BULLET_ASSET_NAME)->mesh.lock()->indexCount);
}

void Game::spawnPlayer() {
    auto player_asset = shapeFactory.createShape(PLAYER_ASSET_NAME);
    player_asset->transform.scale(glm::vec3(0.4f));
    player_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    player_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
    if (auto noise = textureFactory.createTexture("noise").lock()) {
        player_asset->bindTextureBaseColor(noise);
    }
    player = std::make_shared<Player>(Player(std::move(player_asset)));
}

void Game::spawnEmiter(float time_between_shots, glm::vec3 position,
                       glm::vec3 rotation) {
    auto emiter_asset = shapeFactory.createShape(EMMITER_ASSET_NAME);
    emiter_asset->transform.scale(glm::vec3(1.0f));
    emiter_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    if (auto noise = textureFactory.createTexture("noise").lock()) {
        emiter_asset->bindTextureBaseColor(noise);
    }
    emiters.push_back(std::make_shared<emiter>(
        std::move(emiter_asset), currentFrameTime, time_between_shots));
    emiters.back()->setPosition(position);
    emiters.back()->setRotation(rotation);
}

void Game::spawnRandomemiter() {
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
    float radius =
        std::sqrt(AREA_RADIUS_SQ) * 0.8f; // Spawn within 80% of the area radius
    glm::vec3 position =
        glm::vec3(cos(angle) * radius, sin(angle) * radius, 0.f);
    glm::vec3 rotation = glm::vec3(
        0.f, 0.f,
        -(glm::degrees(angle) + 180.0f +
          getRandomFloatBetween(-10.f, 10.f))); // Rotate to face the center
    spawnEmiter(0.5f, position, rotation);
}

void Game::snapPlayerIntoArea() {
    float xpos = player->get_pos().x;
    float ypos = player->get_pos().y;
    float distance_from_center = sqrt(xpos * xpos + ypos * ypos);
    if (distance_from_center > PLAYER_AREA_RADIUS) {
        float scale = PLAYER_AREA_RADIUS / distance_from_center;
        player->setPosition(xpos * scale, ypos * scale, player->get_pos().z);
    }
}

void Game::shootIfTime(float speed) {
    for (auto emiter : emiters) {
        emiter->shootIfTime(shapeFactory, currentFrameTime, speed,
                            bulletBuffer);
    }
}

void Game::moveRemoveBullets() {
    bulletBuffer.moveRemoveActiveElements(deltaTime, player->get_pos());
}

void Game::setupLights() {
    BlinnPhongParameters bpp;
    bpp.num_lights = 3;
    bpp.light_pos[0] = glm::vec3(50.0f, 50.0f, 50.0f);
    bpp.light_color[0] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[0] = 10000.0f;
    bpp.light_pos[1] = glm::vec3(7.0f, 7.0f, 1.0f);
    bpp.light_color[1] = glm::vec3(1.0f, 0.6f, 0.5f);
    bpp.light_strength[1] = 15.0f;
    bpp.light_pos[2] = glm::vec3(-7.0f, -7.0f, 1.0f);
    bpp.light_color[2] = glm::vec3(0.6f, 0.5f, 1.0f);
    bpp.light_strength[2] = 10.0f;
    shader_utils::set_blinn_phong_uniforms(*shaders.gameShader, bpp);
}

void Game::setupAxes() {
    axes[0] = shapeFactory.createShape("cube", glm::vec3(1.f, 0.f, 0.f));
    axes[1] = shapeFactory.createShape("cube", glm::vec3(0.f, 1.f, 0.f));
    axes[2] = shapeFactory.createShape("cube", glm::vec3(0.f, 0.f, 1.f));
    axes[0]->transform.rotate(90.f, glm::vec3(0.f, 1.f, 0.f));
    axes[1]->transform.rotate(-90.f, glm::vec3(1.f, 0.f, 0.f));
    axes[0]->transform.scale(glm::vec3(0.05f, 0.05f, 2.f));
    axes[1]->transform.scale(glm::vec3(0.05f, 0.05f, 2.f));
    axes[2]->transform.scale(glm::vec3(0.05f, 0.05f, 2.f));
    axes[0]->transform.translate(glm::vec3(0.f, 0.f, .5f));
    axes[1]->transform.translate(glm::vec3(0.f, 0.f, .5f));
    axes[2]->transform.translate(glm::vec3(0.f, 0.f, .5f));
}

void Game::setupTable() {
    auto table1 = shapeFactory.createShape("table");
    table1->bindTextureBaseColor(textureFactory.createTexture("wood").lock());
    table1->transform.scale(glm::vec3(16.0f));
    table1->transform.translate(glm::vec3(0.5f, 0.f, -0.715f));
    table1->transform.rotate(90.f, glm::vec3(1.f, 0.f, 0.f));
    shapes.push_back(std::move(table1));
    auto table2 = shapeFactory.createShape("table");
    table2->bindTextureBaseColor(textureFactory.createTexture("wood").lock());
    table2->transform.scale(glm::vec3(16.0f));
    table2->transform.translate(glm::vec3(-0.5f, 0.f, -0.715f));
    table2->transform.rotate(90.f, glm::vec3(1.f, 0.f, 0.f));
    shapes.push_back(std::move(table2));
}

void Game::setupScene() {
    if (settings.benchmarkOn)
        setupBenchmarkScene();
    else
        setupDefaultScene();
}

void Game::setupDefaultScene() {
    loadShaders();
    shaders.gameShader->use();
    setupLights();
    spawnPlayer();
    for (int i = 0; i < 5; ++i) {
        spawnRandomemiter();
    }
    player->setPosition(2.f, 0.f, 0.f);
    setupTable();
    cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                       static_cast<float>(SCR_HEIGHT));
    cam.setPosition(glm::vec3(-10.f, -10.f, 15.f));
    cam.initOrbitForTarget(player->get_pos());
    setupAxes();
}

void Game::setupBenchmarkScene() {
    loadShaders();
    shaders.gameShader->use();
    setupLights();
    spawnPlayer();
    player->setPosition(2.f, 0.f, 0.f);
    setupTable();
    cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                       static_cast<float>(SCR_HEIGHT));
    cam.setPosition(glm::vec3(-10.f, -10.f, 15.f));
    cam.initOrbitForTarget(player->get_pos());
    setupAxes();
}

void Game::updateCamera() {
    cam.orbitAround(player->get_pos());
    if (CONTROLS_MODE == THIRD_PERSON)
        player->setRotation(0.f, 0.f, cam.getYaw());
    shaders.gameShader->use();
    shader_utils::set_blinn_phong_view_pos(*shaders.gameShader,
                                           cam.getPosition());
    shader_utils::set_blinn_phong_camera(*shaders.gameShader, cam.getMatrix());
}

void Game::checkPlayerCollision() {
    if (settings.benchmarkOn)
        return;

    int bulletId = bulletBuffer.checkActiveBulletCollision(player.get());
    if (bulletId != -1) {
        bulletBuffer.deactivateElement(static_cast<size_t>(bulletId));
    }
}

void Game::drawEntities() {
    shaders.gameShader->use();
    player->drawHitbox(*shaders.gameShader);
    player->draw(*shaders.gameShader);
    for (auto &emiter : emiters) {
        emiter->draw(*shaders.gameShader);
    }
    for (auto &shape : shapes) {
        shape->draw(*shaders.gameShader, glm::mat4(1.0f));
    }
    if (!settings.instancingOn)
        bulletBuffer.drawActiveElements(*shaders.gameShader);
    else
        drawBulletsInstanced();
    for (int i = 0; i < 3; ++i) {
        axes[i]->draw(*shaders.gameShader, glm::mat4(1.0f));
    }
}

void Game::drawBulletsInstanced() {
    shaders.instancedShader->use();
    shaders.instancedShader->setMat4("projection", cam.getProjectionMatrix());
    shaders.instancedShader->setMat4("view", cam.getViewMatrix());
    bulletBuffer.drawActiveInstanced(*shaders.instancedShader);
}

void Game::drawText(TextData &text) {
    glDisable(GL_DEPTH_TEST);
    Text.RenderText(text.text, text.x, text.y, text.scale, text.color);
    glEnable(GL_DEPTH_TEST);
}

void Game::bundledDrawText(std::vector<TextData> &texts) {
    glDisable(GL_DEPTH_TEST);
    for (auto &text : texts) {
        Text.RenderText(text.text, text.x, text.y, text.scale, text.color);
    }
    glEnable(GL_DEPTH_TEST);
}

void Game::printStats() {
    int fps = std::round(1.0f / (deltaTime + 0.0001f));

    std::vector<TextData> texts = {
        TextData{.text = std::string("fps: ") + std::to_string(fps),
                 .x = 20.0f,
                 .y = 580.0f,
                 .scale = 0.3f,
                 .color = glm::vec3(0.0f, 0.0f, 0.0f)},
        TextData{.text = std::string("bullets: ") +
                         std::to_string(bulletBuffer.activeElementCount()),
                 .x = 20.0f,
                 .y = 560.0f,
                 .scale = 0.3f,
                 .color = glm::vec3(0.0f, 0.0f, 0.0f)},
        TextData{.text = std::string("draw calls: ") +
                         std::to_string(gameStateData.drawCallsMade),
                 .x = 20.0f,
                 .y = 540.0f,
                 .scale = 0.3f,
                 .color = glm::vec3(0.0f, 0.0f, 0.0f)},
    };

    glDisable(GL_DEPTH_TEST);

    bundledDrawText(texts);

    glEnable(GL_DEPTH_TEST);
}
