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
    shader.setFloat("specularStrength", parameters.specular_strength);
    shader.setFloat("shininess", parameters.shininess);
    shader.setMat4("camera", parameters.camera);
}

void shader_utils::set_blinn_phong_view_pos(const Shader &shader,
                                            const glm::vec3 &viewPos) {
    shader.setVec3("viewPos", viewPos);
}

void shader_utils::set_blinn_phong_camera(const Shader &shader,
                                          const glm::mat4 &camera) {
    shader.setMat4("camera", camera);
}
