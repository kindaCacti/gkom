#ifndef SHADERS_UTILS_H
#define SHADERS_UTILS_H

#include "./shader_s.h"
#include "./blinn_phong.h"

namespace shader_utils {
void load_blinn_phong_uniforms(Shader &shader,
                               BlinnPhongParameters &parameters);
}

#endif