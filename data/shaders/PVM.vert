#version 410

uniform mat4 PVM;

layout(location = 0) in vec3 vPos;

void main()
{
  gl_Position = PVM * vec4(vPos, 1.0);
}

