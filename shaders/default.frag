#version 410

precision highp float;

uniform vec3 diffCol; // diffuse color

layout(location = 0) out vec4 col;

void main()
{
  col = vec4(diffCol, 1.0);
}
