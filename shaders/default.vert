#version 410

uniform mat4 PVM;

layout(location = 0) in vec3 vPos;
layout(location = 2) in vec2 vTexCoords;

out vec2 fTexCoords;

void main()
{
  gl_Position = PVM * vec4(vPos, 1.0);
  fTexCoords = vTexCoords;
}
