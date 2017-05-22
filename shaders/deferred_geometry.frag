#version 410

precision highp float;

uniform sampler2D diffuseMap;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTextureCoordinates;

layout(location = 0) out vec3 gColor;
layout(location = 1) out vec3 gPosition;
layout(location = 2) out vec3 gNormal;

void main() 
{
  gColor    = texture(diffuseMap, fTextureCoordinates).rgb;
  gPosition = fPosition;
  gNormal   = normalize(fNormal);
}
