// instanced.fs
#version 330 core
in vec3 vColor;
in vec3 vFragPos;
in vec3 vNormal;
out vec4 FragColor;

uniform vec3 viewPos;
uniform float ambientStrength;

const int MAX_LIGHTS = 8;
uniform int numLights;
uniform vec3 lightPosArr[MAX_LIGHTS];
uniform vec3 lightColorArr[MAX_LIGHTS];
uniform float lightStrengthArr[MAX_LIGHTS];

const float shininess = 32.0;
const float specularStrength = 0.5;

void main() {
    vec3 albedo = vColor;
    vec3 N = normalize(vNormal);
    vec3 V = normalize(viewPos - vFragPos);

    vec3 sumAmbient = vec3(0.0);
    vec3 sumDiffuse = vec3(0.0);
    vec3 sumSpecular = vec3(0.0);

    int nLights = clamp(numLights, 0, MAX_LIGHTS);
    for (int i = 0; i < nLights; ++i) {
        vec3 lightVec = lightPosArr[i] - vFragPos;
        float dist = length(lightVec);
        float attenuation = 1.0 / (1.0 + dist * dist);
        vec3 L = (dist > 1e-6) ? (lightVec / dist) : vec3(0.0, 0.0, 1.0);

        float diff = max(dot(N, L), 0.0);

        float spec = 0.0;
        if (diff > 0.0) {
            vec3 R = reflect(-L, N);
            spec = pow(max(dot(V, R), 0.0), shininess);
        }

        float strength = max(lightStrengthArr[i], 0.0);
        vec3 lc = lightColorArr[i] * (strength * attenuation);
        vec3 amb = ambientStrength * lc;

        sumAmbient += amb * albedo;
        sumDiffuse += diff * lc * albedo;
        sumSpecular += spec * lc * specularStrength;
    }

    vec3 color = sumAmbient + sumDiffuse + sumSpecular;
    FragColor = vec4(color, 1.0);
}