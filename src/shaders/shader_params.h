#ifndef SHADERS_BLINN_PHONG_H
#define SHADERS_BLINN_PHONG_H

#include <glm/glm.hpp>

#define BASE_COLOR_TEXTURE_UNIT 0

struct BlinnPhongParameters {
    float ambient_strength = 0.15f;
    float specular_strength = 0.5f;
    float shininess = 64.0f;
    int base_color = BASE_COLOR_TEXTURE_UNIT;
    glm::vec3 light_pos = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 light_color = glm::vec3(1.0f);
    glm::mat4 camera = glm::mat4(1.0f);
};

#endif