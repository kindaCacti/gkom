// instanced.vs
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;        // matches cube mesh layout
layout (location = 4) in vec3 iCol0;
layout (location = 5) in vec3 iCol1;
layout (location = 6) in vec3 iCol2;
layout (location = 7) in vec3 iCol3;

uniform mat4 projection;
uniform mat4 view;

out vec3 vColor;
out vec3 vFragPos;
out vec3 vNormal;

void main() {
    vColor = aColor;
    vec3 world = (iCol0 * aPos.x) + (iCol1 * aPos.y) + (iCol2 * aPos.z) + iCol3;
    vFragPos = world;
    // Fast normal transform for instanced bullets.
    // NOTE: This assumes no non-uniform scaling.
    vNormal = mat3(iCol0, iCol1, iCol2) * aNormal;
    gl_Position = projection * view * vec4(world, 1.0);
}