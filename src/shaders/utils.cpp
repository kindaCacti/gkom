#include <glm/glm.hpp>

#include "./utils.h"
#include "./shader_s.h"
#include "./shader_params.h"

void shader_utils::load_blinn_phong_uniforms(Shader &shader,
                                             BlinnPhongParameters &parameters) {
    // Blinn-Phong uniforms
    shader.setInt("tex", parameters.tex);
    shader.setVec3("lightPos", parameters.light_pos);
    shader.setVec3("viewPos", parameters.view_pos);
    shader.setVec3("lightColor", parameters.light_color);
    shader.setVec3("baseColor", parameters.base_color);
    shader.setFloat("ambientStrength", parameters.ambient_strength);
    shader.setFloat("specularStrength", parameters.specular_strength);
    shader.setFloat("shininess", parameters.shininess);
    shader.setMat4("camera", parameters.camera);
}
