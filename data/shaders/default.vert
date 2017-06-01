#version 410

uniform mat4 PVM;
uniform mat4 prevPVM;

layout(location = 0) in vec3 vPos;
layout(location = 2) in vec2 vTexCoords;

out vec2 fTexCoords;
out vec4 fPVMPos;
out vec4 fPrevPVMPos;

void main()
{
  vec4 pos    = vec4(vPos, 1.0);
  fTexCoords  = vTexCoords;
  fPVMPos     = PVM * pos;
  fPrevPVMPos = prevPVM * pos;
  gl_Position = fPVMPos;
}
