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
#include "game.h"

void Game::updateScene() {
    snapPlayerIntoArea();
    removeOutOfBoundsBullets();
    while (currentFrameTime / emiters.size() > 5.f) {
        spawnRandomemiter();
    }
    shootIfTime(BULLET_SPEED);
    moveBullets();
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
    shader = std::make_shared<Shader>("../shaders/blinn-phong.vs",
                                      "../shaders/blinn-phong.fs");
}

void Game::loadAssets() {
    textureFactory.registerTexture(
        std::make_shared<Texture>(Texture::newNoise2D(512, 512)), "noise");
    shapeFactory.registerMesh("../assets/teapot.obj", "teapot",
                              glm::vec3(0.8f, 0.5f, 0.2f));
    shapeFactory.registerCube();
}

void Game::spawnPlayer() {
    auto player_asset = shapeFactory.createShape(PLAYER_ASSET_NAME);
    player_asset->transform.scale(glm::vec3(0.4f));
    player_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    player_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
    if (auto noise = textureFactory.createTexture("noise").lock()) {
        player_asset->bindDiffuseTexture(noise);
    }
    player = std::make_shared<Player>(Player(std::move(player_asset)));
}

void Game::spawnEmiter(float time_between_shots, glm::vec3 position,
                       glm::vec3 rotation) {
    auto emiter_asset = shapeFactory.createShape(EMMITER_ASSET_NAME);
    emiter_asset->transform.scale(glm::vec3(1.0f));
    emiter_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    if (auto noise = textureFactory.createTexture("noise").lock()) {
        emiter_asset->bindDiffuseTexture(noise);
    }
    emiters.push_back(std::make_shared<emiter>(
        std::move(emiter_asset), currentFrameTime, time_between_shots));
    emiters.back()->setPosition(position);
    emiters.back()->setRotation(rotation);
}

void Game::spawnRandomemiter() {
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.14159265f;
    float radius = AREA_RADIUS * 0.8f; // Spawn within 80% of the area radius
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
        auto bullet_pointer =
            emiter->shootIfTime(shapeFactory, currentFrameTime, speed);
        if (bullet_pointer) {
            bullets.push_back(bullet_pointer);
        }
    }
}

void Game::moveBullets() {
    for (auto bullet : bullets) {
        bullet->step(deltaTime, player->get_pos());
    }
}

void Game::removeOutOfBoundsBullets() {
    auto it = bullets.begin();
    while (it != bullets.end()) {
        float xpos = (*it)->get_pos().x;
        float ypos = (*it)->get_pos().y;
        if (sqrt(xpos * xpos + ypos * ypos) > AREA_RADIUS) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::setupDefaultScene() {
    loadShaders();
    shader->use();
    BlinnPhongParameters bpp;
    shader_utils::set_blinn_phong_uniforms(*shader, bpp);
    spawnPlayer();
    for (int i = 0; i < 5; ++i) {
        spawnRandomemiter();
    }
    player->setPosition(2.f, 0.f, 0.f);
    cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                       static_cast<float>(SCR_HEIGHT));
    cam.setPosition(glm::vec3(-10.f, -10.f, 15.f));
    cam.initOrbitForTarget(player->get_pos());
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

void Game::updateCamera() {
    cam.orbitAround(player->get_pos());
    player->setRotation(0.f, 0.f, cam.getYaw());
    shader->use();
    shader_utils::set_blinn_phong_view_pos(*shader, cam.getPosition());
    shader_utils::set_blinn_phong_camera(*shader, cam.getMatrix());
}

void Game::checkPlayerCollision() {
    for (auto bullet : bullets) {
        if (bullet->intersects(&*player)) {
            std::cout << "Player hit!" << static_cast<float>(glfwGetTime())
                      << std::endl;
        }
    }
}

void Game::drawEntities() {
    player->drawHitbox(*shader);
    player->draw(*shader);
    for (auto &emiter : emiters) {
        emiter->draw(*shader);
    }
    for (auto &bullet : bullets) {
        // bullet->drawHitbox(*shader);
        bullet->draw(*shader);
    }
    for (int i = 0; i < 3; ++i) {
        axes[i]->draw(shader->ID, glm::mat4(1.0f));
    }
}

void Game::printStats() {
    std::cout << "Number of bullets: " << bullets.size()
              << " (FPS: " << 1.0f / (deltaTime + 0.0001f) << ")" << "\r"
              << std::flush;
}
