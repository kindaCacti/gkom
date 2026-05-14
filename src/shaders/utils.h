#ifndef SHADERS_UTILS_H
#define SHADERS_UTILS_H

#define BASE_COLOR_TEXTURE_UNIT 0
#define ROUGHNESS_UNIFORM_NAME "roughness"
#define METALLIC_UNIFORM_NAME "metallic"
#define SPECULAR_UNIFORM_NAME "specular"

#include "./shader_s.h"
#include "./shader_params.h"

struct BlinnPhongParameters;

namespace shader_utils {
void set_blinn_phong_uniforms(const Shader &shader,
                              const BlinnPhongParameters &parameters);
void set_blinn_phong_material_uniforms(const Shader &shader, float roughness,
                                       float metallic, float specular);
void set_blinn_phong_camera(const Shader &shader, const glm::mat4 &camera);
void set_blinn_phong_view_pos(const Shader &shader, const glm::vec3 &viewPos);
} // namespace shader_utils

#endif