#version 410

uniform mat4 M;       // model matrix
uniform mat4 PVM;     // model-view-projection matrix
uniform mat4 prevPVM; // previous model-view-projection matrix
uniform mat3 N;       // normal matrix

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTexCoords;

out vec3 fPos;
out vec4 fPVMPos;
out vec4 fPrevPVMPos;
out vec3 fNorm;
out vec2 fTexCoords;

void main() 
{
  vec4 pos    = vec4(vPos, 1.0);
  fPos        = (M * pos).xyz;
  fPVMPos     = PVM * pos;
  fPrevPVMPos = prevPVM * pos;
  fNorm       = N * vNorm;
	fTexCoords  = vTexCoords;
  gl_Position = fPVMPos;
}
