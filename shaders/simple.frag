#version 410

precision highp float;

in vec2 outTextureCoordinates;
out vec4 fragmentColor;
uniform sampler2D colorTexture;

void main() 
{
  fragmentColor = texture(colorTexture, outTextureCoordinates.xy);
}
