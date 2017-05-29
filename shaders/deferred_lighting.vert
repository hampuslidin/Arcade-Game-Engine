#version 410

uniform mat4 PVM;

layout(location = 0) in vec2 vPos;

void main()
{
  gl_Position = PVM * vec4(vPos, 0.0, 1.0);
}

