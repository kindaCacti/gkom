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

const float shininess = 64.0;
const float specularStrength = 0.5;

vec3 fastNorm(vec3 v) {
    float len2 = dot(v, v);
    if (len2 < 1e-12) {
        return vec3(0.0, 0.0, 1.0);
    } else {
        return v * inversesqrt(len2);
    }
}

// Approximate x^64 via squaring.
float fastPow64(float x) {
    float x2 = x * x;
    float x4 = x2 * x2;
    float x8 = x4 * x4;
    float x16 = x8 * x8;
    float x32 = x16 * x16;
    return x32 * x32;
}

void main() {
    vec3 albedo = vColor;
    // Assumes vNormal is provided by the mesh.
    vec3 N = fastNorm(vNormal);
    // Compute view direction V from fragment to camera
    vec3 V = fastNorm(viewPos - vFragPos);

    vec3 sumAmbient = vec3(0.0);
    vec3 sumDiffuse = vec3(0.0);
    vec3 sumSpecular = vec3(0.0);

    int nLights = clamp(numLights, 0, MAX_LIGHTS);
    for (int i = 0; i < nLights; ++i) {
        vec3 lightVec = lightPosArr[i] - vFragPos;
        float dist2 = dot(lightVec, lightVec);
        vec3 L = vec3(0.0, 0.0, 1.0);
        if (dist2 >= 1e-12) {
            L = lightVec * inversesqrt(dist2);
        }
        float attenuation = 1.0 / (1.0 + dist2);

        float diff = max(dot(N, L), 0.0);

        float spec = 0.0;
        if (diff > 0.0) {
            // Classic Phong specular with cheap math:
            // reflect(-L, N) == 2*dot(N,L)*N - L
            vec3 R = (2.0 * diff) * N - L;
            float x = max(dot(V, R), 0.0);
            // shininess is constant 64, so compute x^64 via squaring.
            spec = fastPow64(x);
        }

        float strength = max(lightStrengthArr[i], 0.0);
        vec3 lc = lightColorArr[i] * (strength * attenuation);

        sumAmbient += (ambientStrength * lc);
        sumDiffuse += (diff * lc);
        sumSpecular += (spec * lc);
    }

    vec3 color = (sumAmbient + sumDiffuse) * albedo +
                 (sumSpecular * specularStrength);
    FragColor = vec4(color, 1.0);
}