#version 410

precision highp float;

uniform sampler2D colorTexture;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTextureCoordinates;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gColor;

void main() 
{
  gPosition = fPosition;
  gNormal   = normalize(fNormal);
  gColor    = texture(colorTexture, fTextureCoordinates).rgb;
}
