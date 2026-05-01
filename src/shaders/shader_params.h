#ifndef SHADERS_BLINN_PHONG_H
#define SHADERS_BLINN_PHONG_H

#include <glm/glm.hpp>

#define DIFFUSE_TEXTURE_UNIT 0

struct BlinnPhongParameters {
    int diffuse = DIFFUSE_TEXTURE_UNIT;
    float ambient_strength = 0.15f;
    float specular_strength = 0.5f;
    float shininess = 64.0f;
    glm::vec3 light_pos = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 light_color = glm::vec3(1.0f);
    glm::vec3 base_color = glm::vec3(0.8f, 0.5f, 0.2f);
    glm::mat4 camera = glm::mat4(1.0f);
};

#endif