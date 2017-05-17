#version 410

precision highp float;

uniform sampler2D colorTexture;

in vec2 fTextureCoordinates;

layout(location = 0) out vec4 color;

void main() 
{
  color = texture(colorTexture, fTextureCoordinates);
}
