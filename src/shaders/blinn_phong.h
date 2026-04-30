#ifndef SHADERS_BLINN_PHONG_H
#define SHADERS_BLINN_PHONG_H

#include <glm/glm.hpp>

// shader.setVec3("lightPos", glm::vec3(2.0f, 2.0f, 2.0f));
// shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));
// shader.setVec3("lightColor", glm::vec3(1.0f));
// shader.setVec3("baseColor", glm::vec3(0.8f, 0.5f, 0.2f));
// shader.setFloat("ambientStrength", 0.15f);
// shader.setFloat("specularStrength", 0.5f);
// shader.setFloat("shininess", 64.0f);

struct BlinnPhongParameters {
    float ambient_strength = 0.15f;
    float specular_strength = 0.5f;
    float shininess = 64.0f;
    glm::vec3 light_pos = glm::vec3(2.0f, 2.0f, 2.0f);
    glm::vec3 view_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 light_color = glm::vec3(1.0f);
    glm::vec3 base_color = glm::vec3(0.8f, 0.5f, 0.2f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
};

#endif