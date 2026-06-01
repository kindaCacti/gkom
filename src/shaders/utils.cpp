#include <glm/glm.hpp>

#include <algorithm>
#include <string>

#include "./utils.h"
#include "./shader_s.h"
#include "./shader_params.h"

void shader_utils::set_blinn_phong_uniforms(
    const Shader &shader, const BlinnPhongParameters &parameters) {
    shader.setVec3("viewPos", parameters.view_pos);
    shader.setInt(BASE_COLOR_MAP_UNIFORM_NAME, parameters.base_color);
    shader.setInt(ROUGHNESS_MAP_UNIFORM_NAME, parameters.roughness_map);
    shader.setFloat("ambientStrength", parameters.ambient_strength);
    shader.setFloat(ROUGHNESS_UNIFORM_NAME, parameters.roughness);
    shader.setFloat(SPECULAR_UNIFORM_NAME, parameters.specular);
    shader.setFloat(METALLIC_UNIFORM_NAME, parameters.metallic);

    const int maxLights = BlinnPhongParameters::max_lights;
    const int nLights = std::clamp(parameters.num_lights, 0, maxLights);
    shader.setInt(NUM_LIGHTS_UNIFORM_NAME, nLights);

    // Upload arrays
    shader.setVec3Array(LIGHT_POS_ARR_UNIFORM_NAME, nLights,
                        parameters.light_pos.data());
    shader.setVec3Array(LIGHT_COLOR_ARR_UNIFORM_NAME, nLights,
                        parameters.light_color.data());
    shader.setFloatArray(LIGHT_STRENGTH_ARR_UNIFORM_NAME, nLights,
                         parameters.light_strength.data());

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
