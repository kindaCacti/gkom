#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D textTexture;
uniform vec3 textColor;

void main() {    
    float alphaValue = texture(textTexture, TexCoords).r;
    FragColor = vec4(textColor, alphaValue);
}