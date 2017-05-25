#version 410

precision highp float;

uniform sampler2D diffTexMap;

in vec3 fPos;
in vec3 fNorm;
in vec2 fTexCoords;

layout(location = 0) out vec3 oPos;
layout(location = 1) out vec3 oNorm;
layout(location = 2) out vec3 oCol;
layout(location = 3) out vec2 oVel;

void main() 
{
  oPos  = fPos;
  oNorm = normalize(fNorm);
  oCol  = texture(diffTexMap, fTexCoords).rgb;
}
