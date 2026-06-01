#ifndef SHADERS_BLINN_PHONG_H
#define SHADERS_BLINN_PHONG_H

#include <array>
#include <glm/glm.hpp>
#include "./utils.h"

struct BlinnPhongParameters {
    int base_color = BASE_COLOR_TEXTURE_UNIT;
    int roughness_map = ROUGHNESS_TEXTURE_UNIT;
    float roughness = 0.4f;
    float specular = 0.5f;
    float metallic = 0.0f;
    float ambient_strength = 0.15f;

    static constexpr int max_lights = 8;
    int num_lights = 1;
    std::array<glm::vec3, max_lights> light_pos = {glm::vec3(2.0f, 2.0f, 2.0f)};
    std::array<glm::vec3, max_lights> light_color = {glm::vec3(1.0f)};
    std::array<float, max_lights> light_strength = {1.0f};

    glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::mat4 camera = glm::mat4(1.0f);
};

#endif