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
#include "entities/enemy.h"

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
           (gameSettings.is_benchmark ? BENCHMARK_SPAWNING_NEW_EMMITERS_AFTER_TIME
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
    shaders.gameShader = std::make_shared<Shader>(gameSettings.gameShader.vertexShader.c_str(),
                                                  gameSettings.gameShader.fragmentShader.c_str());
    shaders.textShader =
        std::make_shared<Shader>(gameSettings.textShader.vertexShader.c_str(), gameSettings.textShader.fragmentShader.c_str());
    shaders.instancedShader = std::make_shared<Shader>(
        gameSettings.instancedShader.vertexShader.c_str(), gameSettings.instancedShader.fragmentShader.c_str());
}

void Game::registerMeshAsset(std::string&& name, std::string&& path, Transform&& transform, std::optional<glm::vec3> color) {
    shapeFactory.registerMesh(path, name, color);
    shapeFactory.registerTransform(name, Transform());
}

void Game::loadAssets() {
    textureFactory.registerTexture(
        std::make_shared<Texture>(Texture::newNoise2D(512, 512)), "noise");

    for (const auto& mesh : gameSettings.meshes){
        shapeFactory.registerMesh(mesh.path, mesh.name, mesh.colorOverride);
        shapeFactory.registerTransform(mesh.name, mesh.transform);
    }

    std::cout << gameSettings.textures.size() << std::endl;
    for(const auto& texture : gameSettings.textures) {
        textureFactory.registerTexture(
            std::make_shared<Texture>(Texture::fromFile(texture.path)),
            texture.name
        );
    }

    shapeFactory.registerMesh("../assets/kitchen.obj", "kitchen",
                              glm::vec3(0.8f, 0.5f, 0.2f));
    textureFactory.registerTexture(std::make_shared<Texture>(Texture::fromFile(
                                       "../assets/kitchenDiffuse.png")),
                                   "kitchen_diffuse");
    textureFactory.registerTexture(std::make_shared<Texture>(Texture::fromFile(
                                       "../assets/kitchenRoughness.png")),
                                   "kitchen_roughness");

    shapeFactory.registerCube();

    bulletBuffer.setupInstancedDrawing(
        shapeFactory.createShape(BULLET_ASSET_NAME)->mesh.lock()->VAO,
        shapeFactory.createShape(BULLET_ASSET_NAME)->mesh.lock()->indexCount);
}

void Game::spawnPlayer() {
    auto player_asset = shapeFactory.createShape(PLAYER_ASSET_NAME);
    player_asset->transform.scale(glm::vec3(8.f));
    player_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
    player_asset->transform.rotate(90.f, glm::vec3(0.f, 0.f, 1.f));
    if (auto tex = textureFactory.createTexture("teapot2").lock()) {
        player_asset->bindTextureBaseColor(tex);
    }
    player_asset->setRoughness(0.3f);
    player = std::make_shared<Player>(Player(std::move(player_asset)));
}

void Game::spawnEmiter(float time_between_shots, glm::vec3 position,
                       glm::vec3 rotation) {
    EnemyType type =
        static_cast<EnemyType>(static_cast<float>(rand()) / RAND_MAX * 4.0f);
    emiters.push_back(std::make_shared<Enemy>(
        std::move(shapeFactory.createShape(Enemy::getAssetName(type))), type,
        currentFrameTime, time_between_shots));
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
        glm::degrees(angle) + 180.0f +
            getRandomFloatBetween(-10.f, 10.f));
    spawnEmiter(0.5f, position, rotation);
}

void Game::snapPlayerIntoArea() {
    float xpos = player->get_pos().x;
    float ypos = player->get_pos().y;
    // float distance_from_center = sqrt(xpos * xpos + ypos * ypos);
    // if (distance_from_center > PLAYER_AREA_RADIUS) {
    //     float scale = PLAYER_AREA_RADIUS / distance_from_center;
    //     player->setPosition(xpos * scale, ypos * scale, player->get_pos().z);
    // }
    player->setPosition(
        std::clamp(xpos, -PLAYER_AREA_SIZE_X, PLAYER_AREA_SIZE_X),
        std::clamp(ypos, -PLAYER_AREA_SIZE_Y, PLAYER_AREA_SIZE_Y),
        player->get_pos().z);
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
    bpp.num_lights = 4;
    bpp.light_pos[0] = glm::vec3(-10.f, 20.0f, 10.0f);
    bpp.light_color[0] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[0] = 1000.0f;
    bpp.light_pos[1] = glm::vec3(10.f, 20.0f, 10.0f);
    bpp.light_color[1] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[1] = 1000.0f;
    bpp.light_pos[2] = glm::vec3(-10.f, -20.0f, 10.0f);
    bpp.light_color[2] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[2] = 1000.0f;
    bpp.light_pos[3] = glm::vec3(10.f, -20.0f, 10.0f);
    bpp.light_color[3] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[3] = 1000.0f;

    shaders.gameShader->use();
    shader_utils::set_blinn_phong_uniforms(*shaders.gameShader, bpp);

    // Reuse the same light uniforms for instanced bullets.
    shaders.instancedShader->use();
    shader_utils::set_blinn_phong_uniforms(*shaders.instancedShader, bpp);
}

void Game::setupAxes() {
    axes[0] = shapeFactory.createShape("cube", glm::vec3(1.f, 0.f, 0.f));
    axes[1] = shapeFactory.createShape("cube", glm::vec3(0.f, 1.f, 0.f));
    axes[2] = shapeFactory.createShape("cube", glm::vec3(0.f, 0.f, 1.f));
    axes[0]->transform.rotate(90.f, glm::vec3(0.f, 1.f, 0.f));
    axes[1]->transform.rotate(-90.f, glm::vec3(1.f, 0.f, 0.f));
    axes[0]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[1]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[2]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[0]->transform.translate(glm::vec3(0.f, 0.f, .5f));
    axes[1]->transform.translate(glm::vec3(0.f, 0.f, .5f));
    axes[2]->transform.translate(glm::vec3(0.f, 0.f, .5f));
    axes[3] = shapeFactory.createShape("cube", glm::vec3(1.f, 0.f, 0.f));
    axes[4] = shapeFactory.createShape("cube", glm::vec3(0.f, 1.f, 0.f));
    axes[5] = shapeFactory.createShape("cube", glm::vec3(0.f, 0.f, 1.f));
    axes[3]->transform.rotate(90.f, glm::vec3(0.f, 1.f, 0.f));
    axes[4]->transform.rotate(-90.f, glm::vec3(1.f, 0.f, 0.f));
    axes[3]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[4]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[5]->transform.scale(glm::vec3(0.05f, 0.05f, 1.f));
    axes[3]->transform.translate(glm::vec3(0.f, 0.f, 2.5f));
    axes[4]->transform.translate(glm::vec3(0.f, 0.f, 2.5f));
    axes[5]->transform.translate(glm::vec3(0.f, 0.f, 2.5f));
}

void Game::setupTable() {
    {
        auto kitchen = shapeFactory.createShape("kitchen");
        kitchen->bindTextureBaseColor(
            textureFactory.createTexture("kitchen_diffuse").lock());
        kitchen->bindTextureRoughnessMap(
            textureFactory.createTexture("kitchen_roughness").lock());
        kitchen->transform.scale(glm::vec3(1.5f));
        kitchen->transform.rotate(0.f, glm::vec3(0.f, 0.f, 1.f));
        shapes.push_back(std::move(kitchen));
    }
    {
        auto kitchen = shapeFactory.createShape("kitchen");
        kitchen->bindTextureBaseColor(
            textureFactory.createTexture("kitchen_diffuse").lock());
        kitchen->bindTextureRoughnessMap(
            textureFactory.createTexture("kitchen_roughness").lock());
        kitchen->transform.scale(glm::vec3(-1.5f, -1.5f, 1.5f));
        kitchen->transform.rotate(0.f, glm::vec3(0.f, 0.f, 1.f));
        shapes.push_back(std::move(kitchen));
    }
}

void Game::setupScene() {
    if (gameSettings.is_benchmark)
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
    setupTable();
    cam.setAspectRatio(static_cast<float>(gameSettings.windowWidth) /
                       static_cast<float>(gameSettings.windowHeight));
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
    cam.setAspectRatio(static_cast<float>(gameSettings.windowWidth) /
                       static_cast<float>(gameSettings.windowHeight));
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
    if (gameSettings.is_benchmark)
        return;

    for (int bulletId : bulletBuffer.checkActiveBulletCollision(player.get())) {
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
    if (!gameSettings.is_instanced)
        bulletBuffer.drawActiveElements(*shaders.gameShader);
    else
        drawBulletsInstanced();
    for (int i = 0; i < 6; ++i) {
        axes[i]->draw(*shaders.gameShader, glm::mat4(1.0f));
    }
}

void Game::drawBulletsInstanced() {
    shaders.instancedShader->use();
    shaders.instancedShader->setMat4("projection", cam.getProjectionMatrix());
    shaders.instancedShader->setMat4("view", cam.getViewMatrix());
    shader_utils::set_blinn_phong_view_pos(*shaders.instancedShader,
                                           cam.getPosition());
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
