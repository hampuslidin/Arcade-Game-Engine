#version 410

precision highp float;

uniform sampler2D diffTexMap;

in vec3 fPos;
in vec3 fNorm;
in vec2 fTexCoords;

layout(location = 0) out vec3 gCol;
layout(location = 1) out vec3 gPos;
layout(location = 2) out vec3 gNorm;

void main() 
{
  gCol  = texture(diffTexMap, fTexCoords).rgb;
  gPos  = fPos;
  gNorm = normalize(fNorm);
}
