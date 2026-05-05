// proszę bez jawnych funkcji gl w tej strukturze (ma to być abstrakcja nad
// opengl i glad)

#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>

#include "camera.h"
#include "./entities/player.h"
#include "./entities/emiter.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"
#include "textures/texture_factory.h"
#include "textures/texture.h"
struct Game {
    Camera cam;
    Player player;
    std::vector<Emiter> emiters;
    ShapeFactory shapeFactory;
    TextureFactory textureFactory;
    std::shared_ptr<Shader> shader;
    std::unique_ptr<Shape> axes[3]; // for debugging

    void loadShaders() {
        shader = std::make_shared<Shader>("../shaders/blinn-phong.vs",
                                          "../shaders/blinn-phong.fs");
    }

    void loadAssets() {
        shapeFactory.registerMesh("../assets/teapot.obj", "teapot",
                                  glm::vec3(0.8f, 0.5f, 0.2f));
        shapeFactory.registerCube();
    }

    void spawnPlayer() {
        auto player_asset = shapeFactory.createShape(PLAYER_ASSET_NAME);
        player_asset->transform.scale(glm::vec3(0.4f));
        player_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
        player_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
        if (auto noise = textureFactory.createTexture("noise").lock()) {
            player_asset->bindDiffuseTexture(noise);
        }
        player = Player(std::move(player_asset));
    }

    void spawnEmiter(glm::vec3 position, glm::vec3 scale) {
        auto emiter_asset = shapeFactory.createShape(EMITER_ASSET_NAME);
        emiter_asset->transform.scale(scale);
        emiter_asset->transform.translate(position);
        emiters.push_back(Emiter(std::move(emiter_asset)));
    }

    void setupDefaultScene() {
        loadShaders();
        shader->use();
        BlinnPhongParameters bpp;
        shader_utils::set_blinn_phong_uniforms(*shader, bpp);
        spawnPlayer();
        player.set_position(2.f, 0.f, 0.f);
        cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                           static_cast<float>(SCR_HEIGHT));
        cam.setPosition(glm::vec3(-10.f, -10.f, 15.f));
        cam.initOrbitForTarget(player.get_pos());
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

    void updateCamera() {
        cam.orbitAround(player.get_pos());
        player.set_rotation(0.f, 0.f, cam.getYaw());
        shader->use();
        shader_utils::set_blinn_phong_view_pos(*shader, cam.getPosition());
        shader_utils::set_blinn_phong_camera(*shader, cam.getMatrix());
    }

    void drawEntities() {
        player.draw(*shader);
        for (int i = 0; i < 3; ++i) {
            axes[i]->draw(shader->ID, glm::mat4(1.0f));
        }
    }

    void onFramebufferResize(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        cam.setAspectRatio(static_cast<float>(width) /
                           static_cast<float>(height));
    }

    void onMouseMove(GLFWwindow *window, double xpos, double ypos) {
        cam.onMouseMove(xpos, ypos);
    }
};

#endif