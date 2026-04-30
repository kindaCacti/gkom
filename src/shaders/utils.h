#ifndef SHADERS_UTILS_H
#define SHADERS_UTILS_H

#include "./shader_s.h"
#include "./shader_params.h"

namespace shader_utils {
void set_blinn_phong_uniforms(const Shader &shader,
                              const BlinnPhongParameters &parameters);
void set_blinn_phong_camera(const Shader &shader, const glm::mat4 &camera);
void set_blinn_phong_view_pos(const Shader &shader, const glm::vec3 &viewPos);
} // namespace shader_utils

#endif