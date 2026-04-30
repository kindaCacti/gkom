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

// The texture to use for the material. If the texture is black, it will be ignored and vertex colors or base color will be used instead.
uniform sampler2D tex;

// Used when vertex colors are not provided.
uniform vec3 baseColor;

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main() {
  vec3 albedo = fs_in.Color;
  if (length(albedo) < 1e-6) {
    albedo = baseColor;
  }

  // Texture lookup: prefer mesh UVs, otherwise project from world position.
  vec2 uv = fs_in.TexCoord;
  if (length(uv) < 1e-6) {
    uv = fs_in.FragPos.xz * 0.25;
  }
  uv = fract(uv);
  vec3 texColor = texture(tex, uv).rgb;
  // replace color with texture if texture is not black, otherwise use vertex color or base color
  if (length(texColor) > 1e-6) {
    albedo = texColor;
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
  if (diff > 0.0) {
    spec = pow(max(dot(N, H), 0.0), max(shininess, 1.0));
  }

  vec3 ambient = ambientStrength * lightColor;
  vec3 diffuse = diff * lightColor;
  vec3 specular = specularStrength * spec * lightColor;

  vec3 color = (ambient + diffuse) * albedo + specular;
  FragColor = vec4(color, 1.0);
}
