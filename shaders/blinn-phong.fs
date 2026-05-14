#version 330 core

out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec3 Color;
  vec2 TexCoord;
} fs_in;

uniform vec3 viewPos;
uniform float ambientStrength;

// Multi-light support
const int MAX_LIGHTS = 8;
uniform int numLights;
uniform vec3 lightPosArr[MAX_LIGHTS];
uniform vec3 lightColorArr[MAX_LIGHTS];
uniform float lightStrengthArr[MAX_LIGHTS];

uniform sampler2D baseColor;
uniform float roughness;
uniform float specular;
uniform float metallic;

const float PI = 3.14159265;

void main() {
  vec3 albedo = fs_in.Color;

  // Texture lookup: prefer mesh UVs, otherwise project from world position.
  vec2 uv = fs_in.TexCoord;
  if (length(uv) < 1e-6) {
    uv = fs_in.FragPos.xz * 0.25;
  }
  uv = fract(uv);
  vec3 texBaseColor = texture(baseColor, uv).rgb;
  if (length(texBaseColor) > 1e-6) {
    albedo = texBaseColor;
  }

  vec3 N = fs_in.Normal;
  // If normals are missing, approximate them from screen-space derivatives.
  if (length(N) < 1e-6) {
    vec3 dx = dFdx(fs_in.FragPos);
    vec3 dy = dFdy(fs_in.FragPos);
    N = normalize(cross(dx, dy));
  } else {
    N = normalize(N);
  }

  vec3 V = normalize(viewPos - fs_in.FragPos);

  float r = max(roughness, 0.001);
  // Map roughness -> Blinn-Phong shininess.
  float shininess = clamp(2.0 / (r * r) - 2.0, 1.0, 256.0);

  float m = clamp(metallic, 0.0, 1.0);
  vec3 diffuseAlbedo = albedo * (1.0 - m);
  vec3 F0 = mix(vec3(0.04), albedo, m);
  vec3 specColor = F0 * specular;

  vec3 sumAmbientDiffuse = vec3(0.0);
  vec3 sumAmbientSpecular = vec3(0.0);
  vec3 sumDiffuse = vec3(0.0);
  vec3 sumSpecular = vec3(0.0);

  int nLights = clamp(numLights, 0, MAX_LIGHTS);
  for (int i = 0; i < nLights; ++i) {
    vec3 lightVec = lightPosArr[i] - fs_in.FragPos;
    float dist = length(lightVec);
    // Distance attenuation (stable inverse-square style)
    float attenuation = 1.0 / (1.0 + dist * dist);

    vec3 Li = (dist > 1e-6) ? (lightVec / dist) : vec3(0.0, 0.0, 1.0);
    vec3 Hi = normalize(Li + V);

    float diffi = max(dot(N, Li), 0.0);
    float speci = 0.0;
    if (diffi > 0.0) {
      speci = pow(max(dot(N, Hi), 0.0), shininess);
      speci *= (shininess + 8.0) / (8.0 * PI);
    }

    float strength = max(lightStrengthArr[i], 0.0);
    vec3 lc = lightColorArr[i] * (strength * attenuation);
    vec3 amb = ambientStrength * lc;

    sumAmbientDiffuse += amb * diffuseAlbedo;
    sumAmbientSpecular += amb * specColor;
    sumDiffuse += diffi * lc * diffuseAlbedo;
    sumSpecular += speci * lc * specColor;
  }

  vec3 color = sumAmbientDiffuse + sumDiffuse + sumAmbientSpecular + sumSpecular;
  FragColor = vec4(color, 1.0);
}
