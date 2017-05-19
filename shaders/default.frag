#version 410

precision highp float;

uniform vec3 diffuseColor;

layout(location = 0) out vec3 color;

void main()
{
  color = diffuseColor;
}
