#version 330 core

out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec3 Color;
  vec2 TexCoord;
} fs_in;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float ambientStrength;

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

  vec3 L = normalize(lightPos - fs_in.FragPos);
  vec3 V = normalize(viewPos - fs_in.FragPos);
  vec3 H = normalize(L + V);

  float diff = max(dot(N, L), 0.0);
  float spec = 0.0;
  float r = max(roughness, 0.001);
  // Map roughness -> Blinn-Phong shininess.
  float shininess = clamp(2.0 / (r * r) - 2.0, 1.0, 256.0);
  if (diff > 0.0) {
    spec = pow(max(dot(N, H), 0.0), shininess);
    // Normalized-ish Blinn-Phong (helps specularStrength behave more predictably)
    spec *= (shininess + 8.0) / (8.0 * PI);
  }

  float m = clamp(metallic, 0.0, 1.0);
  vec3 diffuseAlbedo = albedo * (1.0 - m);
  vec3 F0 = mix(vec3(0.04), albedo, m);
  vec3 specColor = F0 * specular;

  vec3 ambient = ambientStrength * lightColor;
  vec3 diffuse = diff * lightColor;
  vec3 specularTerm = specColor * spec * lightColor;

  // Apply the "ambient" term to specular too (cheap stand-in for environment reflections).
  // Without this, metallic surfaces tend to look unnaturally dark in scenes with only direct lighting.
  vec3 ambientDiffuse = ambient * diffuseAlbedo;
  vec3 ambientSpecular = ambient * specColor;

  vec3 color = ambientDiffuse + diffuse * diffuseAlbedo + ambientSpecular + specularTerm;
  FragColor = vec4(color, 1.0);
}
