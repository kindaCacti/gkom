// proszę bez jawnych funkcji gl w tej strukturze (ma to być abstrakcja nad
// opengl i glad)

#ifndef GAME_H
#define GAME_H

#include <memory>

#include "camera.h"
#include "./entities/player.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"

struct Game {
    Camera cam;
    Player player;
    ShapeFactory shapeFactory;
    std::shared_ptr<Shader> shader;

    void loadShaders() {
        shader = std::make_shared<Shader>("../shaders/blinn-phong.vs",
                                          "../shaders/blinn-phong.fs");
    }

    void loadAssets() {
        shapeFactory.registerMesh("../assets/teapot.obj", "teapot",
                                  glm::vec3(0.8f, 0.5f, 0.2f));
    }

    void loadPlayer() {
        auto player_asset = shapeFactory.createShape(PLAYER_ASSET_NAME);
        player_asset->transform.scale(glm::vec3(0.4f));
        player_asset->transform.translate(glm::vec3(0.f, -1.5f, 0.f));
        player = Player(std::move(player_asset));
    }

    void setupDefaultScene() {
        loadShaders();
        shader->use();
        BlinnPhongParameters bpp;
        shader_utils::set_blinn_phong_uniforms(*shader, bpp);
        loadPlayer();
        player.set_position(2.f, 0.f, 0.f);
        cam.setAspectRatio(static_cast<float>(SCR_WIDTH) /
                           static_cast<float>(SCR_HEIGHT));
        cam.setPosition(glm::vec3(0.f, 3.f, 5.f));
    }

    void updateCamera() {
        cam.setTarget(player.get_pos());
        shader->use();
        shader_utils::set_blinn_phong_view_pos(*shader, cam.getPosition());
        shader_utils::set_blinn_phong_camera(*shader, cam.getMatrix());
    }

    void drawEntities() { player.draw(*shader); }
};

#endif