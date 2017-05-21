#version 410

precision highp float;

uniform vec3 diffuseColor;

layout(location = 0) out vec4 color;

void main()
{
  color = vec4(diffuseColor, 1.0);
}
