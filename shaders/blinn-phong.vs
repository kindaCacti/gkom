#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

out VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec3 Color;
  vec2 TexCoord;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  vec4 worldPos = model * vec4(aPos, 1.0);
  vs_out.FragPos = worldPos.xyz;

  mat3 normalMat = mat3(transpose(inverse(model)));
  vs_out.Normal = normalMat * aNormal;

  vs_out.Color = aColor;
  vs_out.TexCoord = aTexCoord;

  gl_Position = projection * view * worldPos;
}
