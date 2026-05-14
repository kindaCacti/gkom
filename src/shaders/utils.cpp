#include <glm/glm.hpp>

#include "./utils.h"
#include "./shader_s.h"
#include "./shader_params.h"

void shader_utils::set_blinn_phong_uniforms(
    const Shader &shader, const BlinnPhongParameters &parameters) {
    shader.setVec3("lightPos", parameters.light_pos);
    shader.setVec3("viewPos", parameters.view_pos);
    shader.setVec3("lightColor", parameters.light_color);
    shader.setInt("baseColor", parameters.base_color);
    shader.setFloat("ambientStrength", parameters.ambient_strength);
    shader.setFloat(ROUGHNESS_UNIFORM_NAME, parameters.roughness);
    shader.setFloat(SPECULAR_UNIFORM_NAME, parameters.specular);
    shader.setFloat(METALLIC_UNIFORM_NAME, parameters.metallic);
    shader.setMat4("camera", parameters.camera);
}

void shader_utils::set_blinn_phong_material_uniforms(const Shader &shader,
                                                     float roughness,
                                                     float metallic,
                                                     float specular) {
    shader.setFloat(ROUGHNESS_UNIFORM_NAME, roughness);
    shader.setFloat(SPECULAR_UNIFORM_NAME, specular);
    shader.setFloat(METALLIC_UNIFORM_NAME, metallic);
}

void shader_utils::set_blinn_phong_view_pos(const Shader &shader,
                                            const glm::vec3 &viewPos) {
    shader.setVec3("viewPos", viewPos);
}

void shader_utils::set_blinn_phong_camera(const Shader &shader,
                                          const glm::mat4 &camera) {
    shader.setMat4("camera", camera);
}
