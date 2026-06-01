// instanced.vs
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;        // matches cube mesh layout
layout (location = 4) in mat4 instanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;
out vec3 vFragPos;
out vec3 vNormal;

void main() {
    vColor = aColor;
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    mat3 normalMat = mat3(transpose(inverse(instanceMatrix)));
    vNormal = normalMat * aNormal;
    gl_Position = projection * view * worldPos;
}