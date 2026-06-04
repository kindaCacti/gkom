#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>
#include <array>
#include <cstdio>

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
#include "entities/plate.h"
#include <iomanip>
#include <sstream>

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
    auto tbe = gameSettings.is_benchmark
                   ? BENCHMARK_SPAWNING_NEW_EMMITERS_AFTER_TIME
                   : SPAWNING_NEW_EMMITERS_AFTER_TIME;
    int spawnedThisFrame = 0;
    const int maxSpawnsPerFrame = 2;
    while (currentFrameTime() - enemies.size() * tbe > 0 &&
           spawnedThisFrame < maxSpawnsPerFrame) {
        spawnRandomEnemy();
        ++spawnedThisFrame;
    }
    shootIfTime(BULLET_SPEED);
    moveRemoveBullets();
    checkBulletCollisions();
    updateCamera();

    expensiveChecks = 0;
    cheapChecks = 0;
    veryCheapChecks = 0;
    for (const auto &plate : plates) {
        expensiveChecks += plate->expensiveCollisionChecks;
        cheapChecks += plate->cheapCollisionChecks;
        plate->resetCollisionCounters();
    }
    expensiveChecks += player->expensiveCollisionChecks;
    cheapChecks += player->cheapCollisionChecks;
    player->resetCollisionCounters();
    expensiveChecks += bulletBuffer.expensiveCollisionChecks;
    veryCheapChecks += bulletBuffer.cheapCollisionChecks;
    bulletBuffer.resetCollisionCounters();
}

void Game::clearNonPlayerEntities() {
    bulletBuffer.clearBuffer();
    enemies.clear();
}

void Game::restartGame() {
    clearNonPlayerEntities();
    restartTimes();
    resetPlayer();
    resetPlates();
}

void Game::drawScene() {
    drawEntities();
    if (settings.showSpawningAreas) {
        SPAWNING_AREAS.drawDebug(shaders.gameShader, *spawningAreaShape);
    }
}

void Game::doFramePreprocessing() { updateTimes(); }

void Game::loadShaders() {
    shaders.gameShader = std::make_shared<Shader>(
        gameSettings.gameShader.vertexShader.c_str(),
        gameSettings.gameShader.fragmentShader.c_str());
    shaders.textShader = std::make_shared<Shader>(
        gameSettings.textShader.vertexShader.c_str(),
        gameSettings.textShader.fragmentShader.c_str());
    shaders.instancedShader = std::make_shared<Shader>(
        gameSettings.instancedShader.vertexShader.c_str(),
        gameSettings.instancedShader.fragmentShader.c_str());
}

void Game::registerMeshAsset(std::string &&name, std::string &&path,
                             Transform &&transform,
                             std::optional<glm::vec3> color) {
    shapeFactory.registerMesh(path, name, color);
    shapeFactory.registerTransform(name, Transform());
}

void Game::loadAssets() {
    textureFactory.registerTexture(
        std::make_shared<Texture>(Texture::newNoise2D(512, 512)), "noise");

    for (const auto &mesh : gameSettings.meshes) {
        shapeFactory.registerMesh(mesh.path, mesh.name, mesh.colorOverride);
        shapeFactory.registerTransform(mesh.name, mesh.transform);
    }

    std::cout << gameSettings.textures.size() << std::endl;
    for (const auto &texture : gameSettings.textures) {
        textureFactory.registerTexture(
            std::make_shared<Texture>(Texture::fromFile(texture.path)),
            texture.name);
    }

    shapeFactory.registerCube();

    shapeFactory.registerWireCube();
    hitboxShape =
        shapeFactory.createShape("hitbox_cube", glm::vec3(0.0f, 1.0f, 0.0f));
    if (hitboxShape) {
        hitboxShape->setRoughness(1.0f);
        hitboxShape->setMetallic(0.0f);
        hitboxShape->setSpecular(0.0f);
    }

    shapeFactory.registerWireCylinder();
    hitboxCylinderShape = shapeFactory.createShape("hitbox_cylinder",
                                                   glm::vec3(0.0f, 1.0f, 0.0f));
    if (hitboxCylinderShape) {
        hitboxCylinderShape->setRoughness(1.0f);
        hitboxCylinderShape->setMetallic(0.0f);
        hitboxCylinderShape->setSpecular(0.0f);
    }

    shapeFactory.registerWireCube("spawning_area_cube");
    spawningAreaShape = shapeFactory.createShape("spawning_area_cube",
                                                 glm::vec3(1.0f, 0.0f, 0.0f));
    if (spawningAreaShape) {
        spawningAreaShape->setRoughness(1.0f);
        spawningAreaShape->setMetallic(0.0f);
        spawningAreaShape->setSpecular(0.0f);
    }

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

void Game::resetPlayer() {
    player->setPosition(0.f, 0.f, 0.f);
    player->setRotation(0.f, 0.f, 0.f);
    switch (settings.difficultyIdx) {
    case 0:
        player->lives = 5;
        break;
    case 1:
        player->lives = 3;
        break;
    case 2:
        player->lives = 1;
        break;
    default:
        player->lives = 3;
        break;
    }
}

void Game::spawnPlates() {
    {
        auto plate_asset = shapeFactory.createShape("plate");
        if (auto tex =
                textureFactory.createTexture("plate_diffuse_green").lock()) {
            plate_asset->bindTextureBaseColor(tex);
        }
        plate_asset->setRoughness(0.3f);
        auto plate = std::make_shared<Plate>(std::move(plate_asset));
        plates.push_back(plate);
    }
    {
        auto plate_asset = shapeFactory.createShape("plate");
        if (auto tex =
                textureFactory.createTexture("plate_diffuse_red").lock()) {
            plate_asset->bindTextureBaseColor(tex);
        }
        plate_asset->setRoughness(0.3f);
        auto plate = std::make_shared<Plate>(std::move(plate_asset));
        plate->setPosition(glm::vec3(5.f, 5.f, 0.f));
        plates.push_back(plate);
    }
    {
        auto plate_asset = shapeFactory.createShape("plate");
        if (auto tex =
                textureFactory.createTexture("plate_diffuse_blue").lock()) {
            plate_asset->bindTextureBaseColor(tex);
        }
        plate_asset->setRoughness(0.3f);
        auto plate = std::make_shared<Plate>(std::move(plate_asset));
        plate->setPosition(glm::vec3(5.f, 5.f, 0.2f));
        plates.push_back(plate);
    }
}

void Game::resetPlates() {
    plates[0]->setPosition(glm::vec3(0.f, 0.f, 0.f));
    plates[0]->setRotation(glm::vec3(0.f, 0.f, 0.f));
    plates[1]->setPosition(glm::vec3(5.f, 5.f, 0.f));
    plates[1]->setRotation(glm::vec3(0.f, 0.f, 0.f));
    plates[2]->setPosition(glm::vec3(-5.f, -5.f, 0.f));
    plates[2]->setRotation(glm::vec3(0.f, 0.f, 0.f));
}

void Game::movePlate(float deltaTime) {
    // find nearest palte to the player
    auto base = player->get_pos() +
                glm::vec3(0.f, 0.f, 1.3f); // slightly above the player
    auto dir = cam.getXYDirection();
    auto nose = base + dir * 1.75f; // point in front of the player where the
                                    // plate should move to
    std::shared_ptr<Plate> plate = currentlyMovingPlate;
    float min_dist_sq = std::numeric_limits<float>::max();
    if (plate) { // if we are already moving a plate, check if it's still close
                 // to the target point
        float dist_sq = glm::distance2(plate->get_pos(), nose);
        if (dist_sq > 4.0f) { // if the plate is too far from the target
                              // point, stop moving it
            stopMovingPlate();
        } else {
            min_dist_sq = dist_sq;
        }
    }
    if (!plate) { // if we are not currently moving a plate, find the closest
                  // one to the target point
        for (const auto &p : plates) {
            float dist_sq = glm::distance2(p->get_pos(), nose);
            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                plate = p;
            }
        }
    }
    if (!plate ||
        min_dist_sq > 3.0f) { // if no plate is close enough, do nothing
        return;
    }
    currentlyMovingPlate = plate;
    // move the plate in front of the palyer as a shield
    auto offset = dir * 1.75f; // distance from player
    auto oldPos = plate->get_pos();
    auto oldRot = plate->get_rot();
    auto newPos = base + offset;
    auto dPos = newPos - oldPos;
    if (glm::length(dPos) > 0.2f) {
        dPos = glm::normalize(dPos) * deltaTime * 20.f;
    }
    // rotate the plate to face the player
    float angle = std::atan2(dir.y, dir.x);
    auto newRot = glm::vec3(-90.f, 0.f, glm::degrees(angle) + 90.f);
    // find shortest rotation direction, taking into account angle wrapping
    auto dRot = newRot - oldRot;
    dRot.x = std::fmod(dRot.x + 180.f, 360.f) - 180.f;
    dRot.y = std::fmod(dRot.y + 180.f, 360.f) - 180.f;
    dRot.z = std::fmod(dRot.z + 180.f, 360.f) - 180.f;
    dRot.x = std::fmod(dRot.x - 180.f, 360.f) + 180.f;
    dRot.y = std::fmod(dRot.y - 180.f, 360.f) + 180.f;
    dRot.z = std::fmod(dRot.z - 180.f, 360.f) + 180.f;
    if (glm::length(dRot) > 3.f) {
        dRot = glm::normalize(dRot) * deltaTime * 400.f;
    }
    plate->setPosition(oldPos + dPos);
    plate->setRotation(oldRot + dRot);
    for (auto other_plate : plates) {
        if (other_plate != plate) {
            // if the plate collides with another plate, move it back to old
            // position
            if (plate->fastCollisionCheck(other_plate.get())) {
                plate->setPosition(oldPos);
                plate->setRotation(oldRot);
                return;
            }
        }
    }
}

void Game::stopMovingPlate() { currentlyMovingPlate = nullptr; }

void Game::spawnEnemy(glm::vec3 position, glm::vec3 rotation) {
    EnemyType type = ORDINARY_COFFEE;
    // weakest type by default, will be upgraded if spawned too close to
    // existing enemy. For random type:
    // EnemyType type =
    //     static_cast<EnemyType>(static_cast<float>(rand()) / RAND_MAX * 4.0f);
    enemies.push_back(std::make_shared<Enemy>(
        shapeFactory.createShape(Enemy::getAssetName(type)), type,
        currentFrameTime()));
    enemies.back()->setPosition(position);
    enemies.back()->setRotation(rotation);
    if (gameSettings.is_benchmark) {
        // For benchmark, spawn stronger enemies to increase difficulty faster.
        enemies.back()->upgrade(shapeFactory, currentFrameTime(), true);
    }
}

void Game::spawnRandomEnemy() {
    glm::vec3 position = SPAWNING_AREAS.randomSample();
    float angle = std::atan2(-position.y, -position.x); // angle towards center
    glm::vec3 rotation = glm::vec3(
        0.f, 0.f, glm::degrees(angle) + getRandomFloatBetween(-10.f, 10.f));
    if (gameSettings.is_benchmark) {
        spawnEnemy(position,
                   rotation); // no rotation for benchmark enemies
        return;
    }
    // avoid spawning too close to existing enemies
    bool too_close = false;
    std::shared_ptr<Enemy> closest_enemy = nullptr;
    for (const auto &enemy : enemies) {
        if (glm::distance2(enemy->get_pos(), position) < 1.75f) {
            too_close = true;
            closest_enemy = enemy;
            break;
        }
    }
    if (too_close && closest_enemy) {
        // skip spawning, upgrade existing instead
        closest_enemy->upgrade(shapeFactory, currentFrameTime());
        return;
    }
    spawnEnemy(position, rotation);
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
    for (auto enemy : enemies) {
        enemy->shootIfTime(shapeFactory, currentFrameTime(), speed,
                           bulletBuffer);
    }
}

void Game::moveRemoveBullets() {
    bulletBuffer.moveRemoveActiveElements(deltaTime(), player->get_pos());
}

void Game::setupLights() {
    BlinnPhongParameters bpp;
    float strength = 500.0f;
    bpp.num_lights = 4;
    bpp.light_pos[0] = glm::vec3(-10.f, 20.0f, 10.0f);
    bpp.light_color[0] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[0] = strength;
    bpp.light_pos[1] = glm::vec3(10.f, 20.0f, 10.0f);
    bpp.light_color[1] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[1] = strength;
    bpp.light_pos[2] = glm::vec3(-10.f, -20.0f, 10.0f);
    bpp.light_color[2] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[2] = strength;
    bpp.light_pos[3] = glm::vec3(10.f, -20.0f, 10.0f);
    bpp.light_color[3] = glm::vec3(1.0f, 1.0f, 1.0f);
    bpp.light_strength[3] = strength;

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
    spawnPlates();
    spawnRandomEnemy();
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
    spawnPlates();
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
    if (CONTROLS_MODE == THIRD_PERSON) {
        player->setRotation(0.f, 0.f, cam.getYaw());
        // check for collision with plates and move player by the smallest
        // amount in the x and y needed to avoid collision
        // for (const auto &plate : plates) {
        //     while (player->check_3D_collision(plate.get())) {
        //         glm::vec3 player_pos = player->get_pos();
        //         glm::vec3 plate_pos = plate->get_pos();
        //         glm::vec3 dir = player_pos - plate_pos;
        //         dir.z = 0.f; // only move in x and y
        //         if (glm::length2(dir) <
        //             0.01f) { // if player is exactly on top of
        //                      // plate, wait for movement input
        //             break;
        //         }
        //         dir = glm::normalize(dir) * 0.05f; // move by a small amount
        //         player->setPosition(player_pos + dir);
        //     }
        // }
    }
    shaders.gameShader->use();
    shader_utils::set_blinn_phong_view_pos(*shaders.gameShader,
                                           cam.getPosition());
    shader_utils::set_blinn_phong_camera(*shaders.gameShader, cam.getMatrix());
}

void Game::checkBulletCollisions() {
    if (!gameSettings.collisionsEnabled)
        return;

    std::vector<HitboxedDrawableEntity *> targets;
    targets.reserve(plates.size() + 1);
    for (auto &plate : plates) {
        targets.push_back(plate.get());
    }
    targets.push_back(player.get());

    const size_t playerIndex = targets.size() - 1;
    const bool deactivateOnHit = !gameSettings.is_benchmark;

    bulletBuffer.collideActiveBulletsFirstHit(
        targets, deactivateOnHit,
        [&](size_t /*bulletIndex*/, size_t targetIndex,
            HitboxedDrawableEntity * /*target*/) {
            if (gameSettings.is_benchmark)
                return;
            if (targetIndex == playerIndex) {
                --player->lives;
            }
        });
}

void Game::drawEntities() {
    shaders.gameShader->use();
    if (hitboxShape && hitboxCylinderShape && settings.showHitboxes) {
        player->drawHitbox(*shaders.gameShader, *hitboxShape,
                           *hitboxCylinderShape);
    }
    player->draw(*shaders.gameShader);
    for (auto &plate : plates) {
        plate->draw(*shaders.gameShader);
        if (hitboxShape && hitboxCylinderShape && settings.showHitboxes) {
            plate->drawHitbox(*shaders.gameShader, *hitboxShape,
                              *hitboxCylinderShape);
        }
    }
    for (auto &enemy : enemies) {
        enemy->draw(*shaders.gameShader);
    }
    for (auto &shape : shapes) {
        shape->draw(*shaders.gameShader, glm::mat4(1.0f));
    }
    if (!gameSettings.is_instanced) {
        bulletBuffer.drawActiveElements(*shaders.gameShader);
        if (hitboxShape && hitboxCylinderShape && settings.showHitboxes) {
            bulletBuffer.drawActiveHitboxes(*shaders.gameShader, *hitboxShape,
                                            *hitboxCylinderShape);
        }
    } else {
        drawBulletsInstanced();
    }
    for (int i = 0; i < 6; ++i) {
        axes[i]->draw(*shaders.gameShader, glm::mat4(1.0f));
    }

    showPlayerLives();
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
    // int fps = std::round(1.0f / (deltaTime() + 0.0001f));

    // static std::array<TextData, 6> texts = {
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 580.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 560.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 540.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 520.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 500.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    //     TextData{.text = "",
    //              .x = 20.0f,
    //              .y = 480.0f,
    //              .scale = 0.3f,
    //              .color = glm::vec3(1.0f)},
    // };

    // // Reuse string capacity to avoid per-frame allocations.
    // texts[0].text.clear();
    // texts[0].text.append("fps: ");
    // texts[0].text.append(std::to_string(fps));

    // texts[1].text.clear();
    // texts[1].text.append("bullets: ");
    // texts[1].text.append(std::to_string(bulletBuffer.activeElementCount()));

    // texts[2].text.clear();
    // texts[2].text.append("draw calls: ");
    // texts[2].text.append(std::to_string(gameStateData.drawCallsMade));

    // texts[3].text.clear();
    // texts[3].text.append("score: ");
    // texts[3].text.append(
    //     std::to_string(static_cast<int>(std::floor(gameplayTime() * 100))));

    // {
    //     const glm::vec3 p = player->get_pos();
    //     char buf[96];
    //     std::snprintf(buf, sizeof(buf), "Player pos: (%.1f, %.1f, %.1f)",
    //     p.x,
    //                   p.y, p.z);
    //     texts[4].text.assign(buf);
    // }

    // {
    //     char buf[96];
    //     std::snprintf(buf, sizeof(buf), "Collisions - E: %d, C: %d, V: %d",
    //                   expensiveChecks, cheapChecks, veryCheapChecks);
    //     texts[5].text.assign(buf);
    // }

    // glDisable(GL_DEPTH_TEST);

    // for (auto &t : texts) {
    //     Text.RenderText(t.text, t.x, t.y, t.scale, t.color);
    // }

    // glEnable(GL_DEPTH_TEST);
}

void Game::showHeart(glm::vec3 pos) {
    auto heart = DrawableEntity(shapeFactory.createShape(HEART_ASSET_NAME));
    heart.setPosition(pos);
    heart.rotate(0.0f, 0.0f, gameplayTime() * 40);
    heart.draw(*shaders.gameShader);
}

void Game::showPlayerLives() {
    float midX = (player->top_x() + player->bottom_x()) / 2.;
    float midY = (player->top_y() + player->bottom_y()) / 2.;
    float midZ = (player->top_z() + player->bottom_z()) / 2. + 1.2f;
    float heartDistance = 1.2f;
    float firstHeartY =
        midY - static_cast<float>(player->lives * heartDistance) / 2.0f + 0.45f;
    glm::vec3 middleHeartPosition(midX, firstHeartY, midZ);
    for (int i = 0; i < player->lives; i++) {
        showHeart(middleHeartPosition);
        middleHeartPosition.y += heartDistance;
    }
}

void Game::showEndScreen() { restartGame(); }
