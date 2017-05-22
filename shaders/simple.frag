#version 410

precision highp float;

uniform sampler2D diffTexMap;

in vec2 fTexCoords;

layout(location = 0) out vec4 col;

void main() 
{
  col = texture(diffTexMap, fTexCoords);
}
