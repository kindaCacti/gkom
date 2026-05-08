// proszę bez jawnych funkcji gl w tej strukturze (ma to być abstrakcja nad
// opengl i glad)

#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>

#include "camera.h"
#include "./entities/player.h"
#include "./entities/emmiter.h"
#include "defines.h"
#include "shaders/shader_s.h"
#include "shaders/utils.h"
#include "textures/texture_factory.h"
#include "textures/texture.h"
#include <list>

struct Game {
    Camera cam;
    std::shared_ptr<Player> player;
    std::list<std::shared_ptr<Emmiter>> emmiters;
    std::list<std::shared_ptr<Bullet>> bullets;
    ShapeFactory shapeFactory;
    TextureFactory textureFactory;
    std::shared_ptr<Shader> shader;
    std::unique_ptr<Shape> axes[3]; // for debugging

    void loadShaders() {
        shader = std::make_shared<Shader>("../shaders/blinn-phong.vs",
                                          "../shaders/blinn-phong.fs");
    }

    void loadAssets() {
        textureFactory.registerTexture(
            std::make_shared<Texture>(Texture::newNoise2D(512, 512)), "noise");
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
        player = std::make_shared<Player>(Player(std::move(player_asset)));
    }

    void spawnEmiter(float current_time, float time_between_shots) {
        auto emmiter_asset = shapeFactory.createShape(EMMITER_ASSET_NAME);
        emmiter_asset->transform.scale(glm::vec3(0.4f));
        emmiter_asset->transform.translate(glm::vec3(0.f, 0.f, 0.f));
        emmiter_asset->transform.rotate(180.f, glm::vec3(0.f, 0.f, 1.f));
        if (auto noise = textureFactory.createTexture("noise").lock()) {
            emmiter_asset->bindDiffuseTexture(noise);
        }
        emmiters.push_back(std::make_shared<Emmiter>(std::move(emmiter_asset), current_time, time_between_shots));
    }

    void make_emmiters_shoot(float current_time, float speed) {
        for(auto emmiter : emmiters) {
            auto bullet_pointer = emmiter->shoot_if_time(shapeFactory, current_time, speed);
            if(bullet_pointer) {
                bullets.push_back(bullet_pointer);
            }
        }
    }

    void move_bullets(float delta_time) {
        for(auto bullet : bullets) {
            bullet->step(delta_time);
        }
    }

    void remove_out_of_bounds_bullets() {
        auto it = bullets.begin();
        while(it != bullets.end()) {
            float xpos = (*it)->get_pos().x;
            float zpos = (*it)->get_pos().z;
            if(xpos < AREA_MIN_X or xpos > AREA_MAX_X or zpos < AREA_MIN_Z or zpos > AREA_MAX_Z) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }

    void setupDefaultScene(float current_time) {
        loadShaders();
        shader->use();
        BlinnPhongParameters bpp;
        shader_utils::set_blinn_phong_uniforms(*shader, bpp);
        spawnPlayer();
        spawnEmiter(current_time, 1.0f);
        player->set_position(2.f, 0.f, 0.f);
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

    void updateCamera() {
        cam.orbitAround(player->get_pos());
        player->set_rotation(0.f, 0.f, cam.getYaw());
        shader->use();
        shader_utils::set_blinn_phong_view_pos(*shader, cam.getPosition());
        shader_utils::set_blinn_phong_camera(*shader, cam.getMatrix());
    }

    void check_collisions() {
        for(auto bullet : bullets) {
            if(bullet->intersects(&*player)) {
                std::cout << "Player hit!" << static_cast<float>(glfwGetTime()) << std::endl;
            }
        }
    }

    void drawEntities() {
        player->drawHitbox(*shader);
        player->draw(*shader);
        for (auto& emiter : emmiters) {
            emiter->draw(*shader);
        }
        for (auto& bullet : bullets) {
            bullet->drawHitbox(*shader);
            // bullet->draw(*shader);
        }
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


    ~Game() = default;
};

#endif