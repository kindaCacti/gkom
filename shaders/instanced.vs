// instanced.vs
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aColor;        // matches cube mesh layout
layout (location = 4) in mat4 instanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}